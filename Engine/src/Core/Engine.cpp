#include "pch.hpp"
#include "Engine.hpp"

#include "Rendering/Window.hpp"

namespace Sphynx {
	void Engine::Init(const EngineInitInfo& info) {
		CrashHandler::Init();
		
		s_Settings.FromInitInfo(info);

		Logging::Init();

		SE_INFO(Logging::General, "=== SPHYNX ENGINE INIT ===");

		if (!s_Settings.Headless) {
			s_Window = new Rendering::Window(s_Settings.WindowName, 1920, 1080, s_Settings.Fullscreen);
		}

		s_UpdateTimer.Reset();
	}

	void Engine::Shutdown() {
		SE_INFO(Logging::General, "=== SPHYNX ENGINE SHUTDOWN ===");

		if (!s_Settings.Headless) {
			delete s_Window;
		}

		Logging::Shutdown();
	}

	void Engine::Update() {
		SE_PROFILE_FUNCTION();

		s_DeltaTime = s_UpdateTimer.ElapsedSeconds();
		s_UpdateTimer.Reset();


		if (!s_Settings.Headless) {
			s_Window->Update();
		}

		if (s_Settings.MaxFPS > 0.f) {
			float updateTime = s_UpdateTimer.ElapsedSeconds();
			float timeLeft = (1.f / s_Settings.MaxFPS) - updateTime;
			if (timeLeft > 0.f) {
				SE_PROFILE_SCOPE("Pit::Engine::MaxFpsLimitWait");
				Time::MicroSleep((uint64_t)(floor(timeLeft * 1000 * 1000))); // Wait for microseconds
			}
		}
	}

	bool Engine::ShouldClose() {
		if (s_Quit.load())
			return true;
		if (!s_Settings.Headless && s_Window && s_Window->ShouldClose())
			return true;
		return false;
	}
}