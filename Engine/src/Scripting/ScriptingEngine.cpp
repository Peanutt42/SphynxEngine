#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Scripting {
	ScriptingEngine::ScriptingEngine() {
		m_Module = std::make_unique<Platform::DynamicLinkLibary>(Engine::GetProject()->BinaryFilepath);
		
		auto isDebugConfigurationFunc = m_Module->LoadFunction<IsDebugConfigurationFunc>("IsDebugConfiguration");
		SE_ASSERT(isDebugConfigurationFunc, Logging::Scripting, "Can't find 'IsDebugConfiguration' function");
		bool isBuiltWithDEBUG = (*isDebugConfigurationFunc)();
#ifdef DEBUG
		SE_ASSERT(isBuiltWithDEBUG, Logging::Scripting, "The Engine was built with DEBUG configuration but not the game module!");
#else
		SE_ASSERT(!isBuiltWithDEBUG, Logging::Scripting, "The Engine wasn't built with DEBUG configuration but the game module was!");
#endif
			

		auto getComponentsFunc = m_Module->LoadFunction<GetComponentsFunc>("GetComponents");
		SE_ASSERT(getComponentsFunc, Logging::Scripting, "Can't find 'GetComponents' function");
		m_Components = (*getComponentsFunc)();
		auto getConfigsFunc = m_Module->LoadFunction<GetConfigsFunc>("GetConfigs");
		SE_ASSERT(getConfigsFunc, Logging::Scripting, "Can't find 'GetConfigs' function");
		m_Configs = (*getConfigsFunc)();
		auto getSystemsFunc = m_Module->LoadFunction<GetSystemsFunc>("GetSystems");
		SE_ASSERT(getSystemsFunc, Logging::Scripting, "Can't find 'GetSystems' function");
		m_Systems = (*getSystemsFunc)();
	}

	ScriptingEngine::~ScriptingEngine() {
		m_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		SE_PROFILE_FUNCTION();

		m_Module->LoadFunction<void(*)()>("TestInput").value()();
	}		
}