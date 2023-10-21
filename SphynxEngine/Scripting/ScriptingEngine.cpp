#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"
#include "ReflectionGenerator/ReflectionInfo.hpp"

namespace Sphynx::Scripting {
	SE_API Platform::DynamicLinkLibrary s_GameModuleDLL;

	void LoadGameModule(const std::filesystem::path& filepath);

	void ScriptingEngine::Init() {
		std::filesystem::path gameModuleFilepath;

		for (auto directory_iter : std::filesystem::directory_iterator(Engine::GetProject().BinariesDirectory)) {
			auto path = directory_iter.path();
			if (Platform::DynamicLinkLibrary::IsDLL(path)) {
				
				auto path_no_extension = path;
				path_no_extension.replace_extension("");
				if (path_no_extension.filename() == Engine::GetProject().GameModuleName)
					gameModuleFilepath = path;
			}
		}
		if (gameModuleFilepath.empty())
			Engine::ForceShutdown(fmt::format("Can't find game module with name: {}", Engine::GetProject().GameModuleName));
	
		LoadGameModule(gameModuleFilepath);
	}
	
	void ScriptingEngine::Shutdown() {
		s_GameModuleDLL.Close();
	}

	void ScriptingEngine::Update() {

	}

	void LoadGameModule(const std::filesystem::path& filepath) {
		SE_INFO(Logging::Scripting, "Loading Game module {}", filepath.string());
		if (!s_GameModuleDLL.Open(filepath))
			Engine::ForceShutdown("Failed to load game module " + s_GameModuleDLL.GetFilepath().string());
	
		bool isDebugConfig = s_GameModuleDLL.Invoke<bool>("IsDebugConfiguration");
#ifdef DEBUG
		if (!isDebugConfig)
			Engine::ForceShutdown("The Engine is built with Debug, but the game isn't!");
#else
		if (isDebugConfig)
			Engine::ForceShutdown("The Engine isn't built with Debug, but the game is!");
#endif

		auto components = s_GameModuleDLL.Invoke<std::vector<Sphynx::ReflectionGenerator::ComponentReflectionInfo>*>("GetComponents");
		for (const auto& component : *components) {
			SE_INFO(Logging::Scripting, "{}", component.Fullname);
			for (const auto& var : component.Variables)
				SE_INFO(Logging::Scripting, "\t{} {}", var.Type, var.Name);
		}
	}
}