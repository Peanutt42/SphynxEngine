#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Scripting {
	using GetComponentsFunc = std::vector<ComponentReflectionInfo>* (*)();
	using GetConfigsFunc = std::vector<ConfigReflectionInfo>* (*)();
	using GetSystemsFunc = std::vector<SystemReflectionInfo>* (*)();

	using IsDebugConfigurationFunc = bool(*)();


	SE_API std::unique_ptr<Platform::DynamicLinkLibary> s_Module;
	SE_API std::vector<ComponentReflectionInfo>* s_Components = nullptr;
	SE_API std::vector<ConfigReflectionInfo>* s_Configs = nullptr;
	SE_API std::vector<SystemReflectionInfo>* s_Systems = nullptr;


	bool ScriptingEngine::Init() {
		std::filesystem::path copiedDLLPath = Engine::GetProject().BinaryFilepath.filename();
		std::error_code error;
		std::filesystem::copy(Engine::GetProject().BinaryFilepath, copiedDLLPath, std::filesystem::copy_options::update_existing, error);
		if (error) {
			SE_ERR("Failed to copy game module {} to {}", std::filesystem::absolute(Engine::GetProject().BinaryFilepath).string(), std::filesystem::absolute(copiedDLLPath).string());
			return false;
		}
		
		s_Module = std::make_unique<Platform::DynamicLinkLibary>(copiedDLLPath);

		auto isDebugConfigurationFunc = s_Module->LoadFunction<IsDebugConfigurationFunc>("IsDebugConfiguration");
		if (!isDebugConfigurationFunc) {
			SE_ERR("Can't find 'IsDebugConfiguration' function");
			return false;
		}

		bool isBuiltWithDEBUG = (*isDebugConfigurationFunc)();
#ifdef DEBUG
		if (!isBuiltWithDEBUG) {
			SE_ERR("The Engine was built with DEBUG configuration but not the game module!");
			return false;
		}
#else
		if (isBuiltWithDEBUG) {
			SE_ERR("The Engine wasn't built with DEBUG configuration but the game module was!");
			return false;
		}
#endif
			
#define LOAD_FUNCTION(name, funcType, outResult)	\
		auto outResult##Func = s_Module->LoadFunction<funcType>(name);	\
		if (!outResult##Func) {											\
			SE_ERR("Can't find '" name "' function");					\
			return false;												\
		}																\
		outResult = (*outResult##Func)();


		LOAD_FUNCTION("GetComponents", GetComponentsFunc, s_Components);
		LOAD_FUNCTION("GetConfigs", GetConfigsFunc, s_Configs);
		LOAD_FUNCTION("GetSystems", GetSystemsFunc, s_Systems);
#undef LOAD_FUNCTION

		return true;
	}

	void ScriptingEngine::Shutdown() {
		s_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		SE_PROFILE_FUNCTION();

	}

	const std::vector<ComponentReflectionInfo>& ScriptingEngine::GetComponents() { return *s_Components; }
	const std::vector<ConfigReflectionInfo>& ScriptingEngine::GetConfigs() { return *s_Configs; }
	const std::vector<SystemReflectionInfo>& ScriptingEngine::GetSystems() { return *s_Systems; }
}