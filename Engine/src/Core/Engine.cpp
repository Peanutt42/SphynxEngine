#include "pch.hpp"
#include "Engine.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/Window.hpp"
#include "Scripting/ScriptingEngine.hpp"
#include "UI/VulkanImGuiHelper.hpp"

namespace Sphynx {
	void Engine::Init(const EngineInitInfo& initInfo) {
		CrashHandler::Init();

		s_Settings = initInfo.Settings;

		Logging::Init();

		SE_INFO("=== SPHYNX ENGINE INIT ===");

		s_Project = initInfo.Project;

		s_Application = initInfo.Application;

		s_ScriptingEngine = new Scripting::ScriptingEngine();

		if (!s_Settings.Headless) {
			s_Window = new Rendering::Window(s_Settings.WindowName, 1920, 1080, s_Settings.Fullscreen);

			Input::Init(s_Window->GetGLFWHandle());

			s_Renderer = new Rendering::Renderer(*s_Window, &Update);

			if (s_Settings.ImGuiEnabled)
				s_ImGuiHelper = new UI::VulkanImGuiHelper();
		}

		s_UpdateTimer.Reset();


		s_Application->OnCreate();
	}

	void Engine::Shutdown() {
		SE_INFO("=== SPHYNX ENGINE SHUTDOWN ===");

		s_Application->OnDestroy();

		if (!s_Settings.Headless)
			s_Renderer->WaitBeforeClose();
		
		s_Project.reset();

		delete s_ScriptingEngine;

		if (!s_Settings.Headless) {
			if (s_Settings.ImGuiEnabled)
				delete s_ImGuiHelper;

			delete s_Renderer;
			delete s_Window;
		}

		Logging::Shutdown();
	}

	void Engine::Update() {
		SE_PROFILE_FUNCTION();

		s_DeltaTime = s_UpdateTimer.ElapsedSeconds();
		s_UpdateTimer.Reset();

		if (!s_Settings.Headless)
			Input::Update();

		s_ScriptingEngine->Update();

		s_Application->Update();

		if (!s_Settings.Headless) {
			if (s_Settings.ImGuiEnabled) {
				s_ImGuiHelper->Begin();
				s_Application->DrawUI();
				s_ImGuiHelper->End();
			}
			
			s_Renderer->Begin();
			s_ImGuiHelper->Render();
			s_Renderer->End();

			s_Window->Update();
		}

		if (s_Settings.MaxFPS > 0.f) {
			float updateTime = s_UpdateTimer.ElapsedSeconds();
			float timeLeft = (1.f / s_Settings.MaxFPS) - updateTime;
			if (timeLeft > 0.f) {
				SE_PROFILE_SCOPE("Sphynx::Engine::MaxFpsCap");
				Time::MicroSleep((uint64_t)(floor(timeLeft * 1000 * 1000))); // Wait for microseconds
			}
		}

		SE_PROFILE_FRAME_END("Main Thread");
	}

	bool Engine::ShouldClose() {
		if (s_Quit.load())
			return true;
		if (!s_Settings.Headless && s_Window && s_Window->ShouldClose())
			return true;
		return false;
	}
}