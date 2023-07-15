#pragma once

#include "Core/CoreInclude.hpp"
#include "ScriptingModule.hpp"
#include "../../Programs/ReflectionGenerator/src/ReflectionInfo.hpp"

namespace Sphynx::Scripting {
	class ScriptingEngine {
	public:
		ScriptingEngine();
		~ScriptingEngine();

		const std::vector<ComponentReflectionInfo>& GetComponents() const { return *m_Components; }
		const std::vector<ConfigReflectionInfo>& GetConfigs() const { return *m_Configs; }
		const std::vector<SystemReflectionInfo>& GetSystems() const { return *m_Systems; }

	private:
		using GetComponentsFunc = std::vector<ComponentReflectionInfo>* (*)();
		using GetConfigsFunc = std::vector<ConfigReflectionInfo>* (*)();
		using GetSystemsFunc = std::vector<SystemReflectionInfo>* (*)();

	private:
		std::unique_ptr<Module> m_Module;
		std::vector<ComponentReflectionInfo>* m_Components = nullptr;
		std::vector<ConfigReflectionInfo>* m_Configs = nullptr;
		std::vector<SystemReflectionInfo>* m_Systems = nullptr;
	};
}