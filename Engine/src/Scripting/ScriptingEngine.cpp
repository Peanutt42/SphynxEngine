#include "pch.hpp"
#include "ScriptingEngine.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Scripting {
	ScriptingEngine::ScriptingEngine() {
		m_Module = std::make_unique<Platform::DynamicLinkLibary>(Engine::GetProject()->Folderpath / "Binaries/" / (Engine::GetProject()->Name + ".dll"));
		m_Components = m_Module->LoadFunction<GetComponentsFunc>("GetComponents")();
		m_Configs = m_Module->LoadFunction<GetConfigsFunc>("GetConfigs")();
		m_Systems = m_Module->LoadFunction<GetSystemsFunc>("GetSystems")();

		// Testing
		for (const ComponentReflectionInfo& info : *m_Components) {
			std::cout << "\t" << info.FullName << '\n';
		}
	}

	ScriptingEngine::~ScriptingEngine() {
		m_Module.reset();
	}
	
	void ScriptingEngine::Update() {
		m_Module->LoadFunction<void(*)()>("TestInput")();
	}
}