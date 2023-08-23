#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Scripting {
	ScriptingEngine::ScriptingEngine() {
		m_Module = std::make_unique<Platform::DynamicLinkLibary>(Engine::GetProject()->Folderpath / "Binaries/" / (Engine::GetProject()->Name + ".dll"));
		auto getComponentsFunc = m_Module->LoadFunction<GetComponentsFunc>("GetComponents");
		SE_ASSERT(getComponentsFunc, Logging::Scripting, "Can't find 'GetComponents' function");
		m_Components = getComponentsFunc.value()();
		auto getConfigs = m_Module->LoadFunction<GetConfigsFunc>("GetConfigs");
		SE_ASSERT(getConfigs, Logging::Scripting, "Can't find 'GetConfigs' function");
		m_Configs = getConfigs.value()();
		auto getSystems = m_Module->LoadFunction<GetSystemsFunc>("GetSystems");
		SE_ASSERT(getSystems, Logging::Scripting, "Can't find 'GetSystems' function");
		m_Systems = getSystems.value()();
	}

	ScriptingEngine::~ScriptingEngine() {
		m_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		SE_PROFILE_FUNCTION();

		m_Module->LoadFunction<void(*)()>("TestInput").value()();
	}		
}