#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Scripting {
	ScriptingEngine::ScriptingEngine() {
		std::filesystem::path copiedDLLPath = Engine::GetProject().BinaryFilepath.filename();
		std::error_code error;
		std::filesystem::copy(Engine::GetProject().BinaryFilepath, copiedDLLPath, std::filesystem::copy_options::update_existing, error);
		if (error)
			Engine::ForceShutdown(true, std::format("Failed to copy game module {} to {}", std::filesystem::absolute(Engine::GetProject().BinaryFilepath).string(), std::filesystem::absolute(copiedDLLPath).string()));
		
		m_Module = std::make_unique<Platform::DynamicLinkLibary>(copiedDLLPath);
		
		auto isDebugConfigurationFunc = m_Module->LoadFunction<IsDebugConfigurationFunc>("IsDebugConfiguration");
		if (!isDebugConfigurationFunc) Engine::ForceShutdown(true, "Can't find 'IsDebugConfiguration' function");
		bool isBuiltWithDEBUG = (*isDebugConfigurationFunc)();
#ifdef DEBUG
		if (!isBuiltWithDEBUG)
			Engine::ForceShutdown(true, "The Engine was built with DEBUG configuration but not the game module!");
#else
		if (isBuiltWithDEBUG)
			Engine::ForceShutdown(true, "The Engine wasn't built with DEBUG configuration but the game module was!");
#endif
			

		auto getComponentsFunc = m_Module->LoadFunction<GetComponentsFunc>("GetComponents");
		if (!getComponentsFunc) Engine::ForceShutdown(true, "Can't find 'GetComponents' function");
		m_Components = (*getComponentsFunc)();

		auto getConfigsFunc = m_Module->LoadFunction<GetConfigsFunc>("GetConfigs");
		if (!getConfigsFunc) Engine::ForceShutdown(true, "Can't find 'GetConfigs' function");
		m_Configs = (*getConfigsFunc)();

		auto getSystemsFunc = m_Module->LoadFunction<GetSystemsFunc>("GetSystems");
		if (!getSystemsFunc) Engine::ForceShutdown(true, "Can't find 'GetSystems' function");
		m_Systems = (*getSystemsFunc)();
	}

	ScriptingEngine::~ScriptingEngine() {
		m_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		SE_PROFILE_FUNCTION();

	}		
}