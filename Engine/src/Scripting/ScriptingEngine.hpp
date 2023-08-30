#pragma once

#include "pch.hpp"
#include "../../Programs/ReflectionGenerator/src/ReflectionInfo.hpp"

namespace Sphynx::Scripting {
	class SE_API ScriptingEngine {
	public:
		ScriptingEngine();
		~ScriptingEngine();

		void Update();

		const std::vector<ComponentReflectionInfo>& GetComponents() const { return *m_Components; }
		const std::vector<ConfigReflectionInfo>& GetConfigs() const { return *m_Configs; }
		const std::vector<SystemReflectionInfo>& GetSystems() const { return *m_Systems; }

	private:
		using GetComponentsFunc = std::vector<ComponentReflectionInfo>*(*)();
		using GetConfigsFunc = std::vector<ConfigReflectionInfo>*(*)();
		using GetSystemsFunc = std::vector<SystemReflectionInfo>*(*)();

		using IsDebugConfigurationFunc = bool(*)();

	private:
		std::unique_ptr<Platform::DynamicLinkLibary> m_Module;
		std::vector<ComponentReflectionInfo>* m_Components = nullptr;
		std::vector<ConfigReflectionInfo>* m_Configs = nullptr;
		std::vector<SystemReflectionInfo>* m_Systems = nullptr;
	};
}