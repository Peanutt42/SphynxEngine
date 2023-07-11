#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"
#include "EngineSettings.hpp"
#include "ProjectSystem/Project.hpp"

namespace Sphynx {
	namespace Rendering {
		class Window;
		class Renderer;
	}

	class Engine {
	public:
		static void Init(const EngineInitInfo& initInfo);
		static void Shutdown();

		static void Update();

		static void CloseNextFrame() { s_Quit.store(true); }

		static bool ShouldClose();

		static float DeltaTime() { return s_DeltaTime; }

		static std::shared_ptr<Project> GetProject() { return s_Project; }

		constexpr static Version Version { 0, 0, 1 };

	private:
		inline static std::atomic_bool s_Quit = false;

		inline static EngineSettings s_Settings;

		inline static float s_DeltaTime = 0.f;
		inline static Timer s_UpdateTimer;

		inline static std::shared_ptr<Project> s_Project;

		inline static Rendering::Window* s_Window = nullptr;
		inline static Rendering::Renderer* s_Renderer = nullptr;
	};
}