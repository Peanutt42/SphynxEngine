#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"
#include "EngineSettings.hpp"

namespace Sphynx {
	namespace Rendering {
		class Window;
	}

	class Engine {
	public:
		static void Init(const EngineInitInfo& info);
		static void Shutdown();

		static void Update();

		static void CloseNextFrame() { s_Quit.store(true); }

		static bool ShouldClose();

		static float DeltaTime() { return s_DeltaTime; }

	private:
		inline static std::atomic_bool s_Quit = false;

		inline static EngineSettings s_Settings;

		inline static float s_DeltaTime = 0.f;
		inline static Timer s_UpdateTimer;

		inline static Rendering::Window* s_Window = nullptr;
	};
}