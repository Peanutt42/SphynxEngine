#pragma once

#include "CoreInclude.hpp"
#include "EngineSettings.hpp"
#include "ProjectSystem/Project.hpp"

namespace Sphynx {
	class SE_API Engine {
	public:
		static void Init(const EngineSettings& settings, Project& project, Application& application);
		static void Shutdown();

		static void Update();

		static void CloseNextFrame();

		// this is for user errors, not engine internal crashes
		static void ForceShutdown();
		// this is for user errors, not engine internal crashes
		static void ForceShutdown(std::string_view msg);

		static bool ShouldClose();

		static float DeltaTime();

		static EngineSettings& GetSettings();

		static Project& GetProject();

		constexpr static Version Version { 0, 0, 1 };
	};
}