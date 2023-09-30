#pragma once

#include "pch.hpp"
#include "../Programs/ReflectionGenerator/ReflectionInfo.hpp"

namespace Sphynx::Scripting {
	class SE_API ScriptingEngine {
	public:
		static void Init();
		static void Shutdown();

		static void Update();

		static const std::vector<ComponentReflectionInfo>& GetComponents();
		static const std::vector<ConfigReflectionInfo>& GetConfigs();
		static const std::vector<SystemReflectionInfo>& GetSystems();
	};
}