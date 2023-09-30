#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

#include "FileWatch.hpp"

namespace Sphynx::Scripting {
	using GetComponentsFunc = std::vector<ComponentReflectionInfo>* (*)();
	using GetConfigsFunc = std::vector<ConfigReflectionInfo>*(*)();
	using GetSystemsFunc = std::vector<SystemReflectionInfo>*(*)();

	using IsDebugConfigurationFunc = bool(*)();


	SE_API std::unique_ptr<Platform::DynamicLinkLibary> s_Module;
	SE_API std::vector<ComponentReflectionInfo> s_Components;
	SE_API std::vector<ConfigReflectionInfo> s_Configs;
	SE_API std::vector<SystemReflectionInfo> s_Systems;

	SE_API std::unique_ptr<filewatch::FileWatch<std::string>> s_BinaryFolderWatch;
	SE_API std::filesystem::path s_LoadedBinaryFilepath;
	SE_API std::filesystem::path s_LatestBinaryFilepath;
	SE_API std::atomic_bool s_ReloadBinaries = false;

	void LoadModule();
	
	// T should be a std::vector<>
	template<typename T>
	bool AreEquivalent(const T& left, const T& right) {
		for (const auto& i : left) {
			bool foundEquivalent = false;
			for (const auto& j : right) {
				if (i.IsEquivalent(j)) {
					foundEquivalent = true;
					break;
				}
			}
			if (!foundEquivalent)
				return false;
		}
		return true;
	}

	void RemoveOldGameFiles(const std::filesystem::path& persistentFilename);

	void CopyGameModule(const std::filesystem::path& binaryFilepath);

	void ScriptingEngine::Init() {
		s_LoadedBinaryFilepath = Engine::GetProject().Name + Platform::DynamicLinkLibary::DLLExtension();

		std::filesystem::path latestBinary;
		for (auto it : std::filesystem::directory_iterator(Engine::GetProject().BinariesDir)) {
			auto path = it.path();
			if (it.is_directory() || !Platform::DynamicLinkLibary::IsDLL(path))
				continue;
			
			if (std::filesystem::exists(latestBinary)) {
				if (std::filesystem::last_write_time(path) > std::filesystem::last_write_time(latestBinary))
					latestBinary = path;
			}
			else
				latestBinary = path;
		}
		if (!std::filesystem::exists(latestBinary))
			Engine::ForceShutdown(true, "Failed to find a usable game binary file");

		

		CopyGameModule(latestBinary);
		auto preservedName = latestBinary.filename();
		preservedName.replace_extension("");
		RemoveOldGameFiles(preservedName);
		SE_INFO(Logging::Scripting, "Loading game binary: {}", std::filesystem::relative(latestBinary, Engine::GetProject().Folderpath).string());
		s_Module = std::make_unique<Platform::DynamicLinkLibary>(s_LoadedBinaryFilepath);
		LoadModule();
		

		s_BinaryFolderWatch = std::make_unique<filewatch::FileWatch<std::string>>(
			Engine::GetProject().BinariesDir.string(),
			[](const std::string& path, const filewatch::Event eventType) {
				std::filesystem::path absolutePath = Engine::GetProject().BinariesDir / path;
				if (absolutePath.parent_path() != Engine::GetProject().BinariesDir)
					return;

				if (eventType == filewatch::Event::added) {
					if (path == "BuildFinished")
						s_ReloadBinaries.store(true);
					else if (Platform::DynamicLinkLibary::IsDLL(path))
						s_LatestBinaryFilepath = absolutePath;
				}
			}
		);
	}

	void ScriptingEngine::Shutdown() {
		s_BinaryFolderWatch.reset();
		s_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		SE_PROFILE_FUNCTION();

		if (s_ReloadBinaries.load()) {
			s_Module.reset();
			CopyGameModule(s_LatestBinaryFilepath);
			std::vector<ComponentReflectionInfo> oldComponents = s_Components;
			std::vector<ConfigReflectionInfo> oldConfigs = s_Configs;
			std::vector<SystemReflectionInfo> oldSystems = s_Systems;
			s_Module = std::make_unique<Platform::DynamicLinkLibary>(s_LoadedBinaryFilepath);
			LoadModule();
			auto preservedName = s_LatestBinaryFilepath.filename();
			preservedName.replace_extension("");
			RemoveOldGameFiles(preservedName);

			// TODO: Implement reloading of components/configs
			if (!AreEquivalent(s_Components, oldComponents) || !AreEquivalent(s_Configs, oldConfigs))
				Engine::ForceShutdown(false, "TODO: Implement reloading of components/configs");

			s_ReloadBinaries.store(false);
		}
	}

	void LoadModule() {
		auto isDebugConfigurationFunc = s_Module->LoadFunction<IsDebugConfigurationFunc>("IsDebugConfiguration");
		if (!isDebugConfigurationFunc)
			Engine::ForceShutdown(true, "Can't find 'IsDebugConfiguration' function");


		bool isBuiltWithDEBUG = (*isDebugConfigurationFunc)();
#ifdef DEBUG
		if (!isBuiltWithDEBUG)
			Engine::ForceShutdown(true, "The Engine was built with DEBUG configuration but not the game module!");
#else
		if (isBuiltWithDEBUG)
			Engine::ForceShutdown(true, "The Engine wasn't built with DEBUG configuration but the game module was!");
#endif

		if (auto componentsFunc = s_Module->LoadFunction<GetComponentsFunc>("GetComponents"))
			s_Components = *(*componentsFunc)();
		else
			Engine::ForceShutdown(true, "Can't find 'GetComponents' function");

		if (auto configsFunc = s_Module->LoadFunction<GetConfigsFunc>("GetConfigs"))
			s_Configs = *(*configsFunc)();
		else
			Engine::ForceShutdown(true, "Can't find 'GetConfigs' function");

		if (auto systemsFunc = s_Module->LoadFunction<GetSystemsFunc>("GetSystems"))
			s_Systems = *(*systemsFunc)();
		else
			Engine::ForceShutdown(true, "Can't find 'GetSystems' function");
	}

	void RemoveOldGameFiles(const std::filesystem::path& persistentFilename) {
		for (auto it : std::filesystem::directory_iterator(Engine::GetProject().BinariesDir)) {
			auto path = it.path();
			auto extension = path.extension();
			if (extension != L".pdb")
				continue;

			auto pathFilename = path.filename();
			pathFilename.replace_extension("");
			if (pathFilename == persistentFilename)
				continue;

			std::error_code error;
			std::filesystem::remove(path, error);
			if (error)
				SE_INFO("Failed to remove {}, {}", path.string(), error.message());
		}
	}

	void CopyGameModule(const std::filesystem::path& binaryFilepath) {
		std::error_code error;
		std::filesystem::copy(binaryFilepath, s_LoadedBinaryFilepath, std::filesystem::copy_options::update_existing, error);
		if (error)
			Engine::ForceShutdown(true, std::format("Failed to copy game module {} to {}", binaryFilepath.string(), s_LoadedBinaryFilepath.string()));
	}

	const std::vector<ComponentReflectionInfo>& ScriptingEngine::GetComponents() { return s_Components; }
	const std::vector<ConfigReflectionInfo>& ScriptingEngine::GetConfigs() { return s_Configs; }
	const std::vector<SystemReflectionInfo>& ScriptingEngine::GetSystems() { return s_Systems; }
}