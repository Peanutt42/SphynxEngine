#include "pch.hpp"
#include "Engine.hpp"
#include "CommandHandler.hpp"

#include "Audio/AudioEngine.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/Window.hpp"
#include "Physics/PhysicEngine.hpp"
#include "Scripting/ScriptingEngine.hpp"
#include "UI/VulkanImGuiHelper.hpp"

namespace Sphynx {
	void Engine::Init(const EngineSettings& settings, Project& project, Application& application) {
		CrashHandler::Init();
		CrashHandler::StartCrashReporter();

		s_Settings = settings;

		Logging::Init();

		SE_INFO("=== SPHYNX ENGINE INIT ===");

		s_Project = &project;

		s_Application = &application;

		s_PhysicEngine = new Physics::PhysicEngine();

		s_ScriptingEngine = new Scripting::ScriptingEngine();

		if (s_Settings.Headless) {
			ConsoleInput::Init();
			ConsoleInput::SetInputCallback([](const std::string& command) {
				CommandHandler::QueueCommand(command);
			});
		}
		else {
			s_AudioEngine = new Audio::AudioEngine();

			s_Window = new Rendering::Window(s_Settings.WindowName, true, s_Settings.Fullscreen, s_Settings.CustomWindowControls);

			Input::Init(s_Window->GetGLFWHandle());

			s_Renderer = new Rendering::Renderer(*s_Window, &Update);

			if (s_Settings.ImGuiEnabled)
				s_ImGuiHelper = new UI::VulkanImGuiHelper();
		}

		s_Application->OnCreate();

		s_UpdateTimer.Reset();
	}

	void Engine::Shutdown() {
		s_Application->OnDestroy();

		if (!s_Settings.Headless && s_Renderer)
			s_Renderer->WaitBeforeClose();

		delete s_ScriptingEngine;

		delete s_PhysicEngine;

		if (s_Settings.Headless)
			ConsoleInput::Shutdown();
		else {
			if (s_Settings.ImGuiEnabled)
				delete s_ImGuiHelper;

			delete s_Renderer;
			delete s_Window;

			delete s_AudioEngine;
		}

		SE_INFO("=== SPHYNX ENGINE SHUTDOWN ===");

		Logging::Shutdown();
	}

	void Engine::Update() {
		SE_PROFILE_FUNCTION();

		s_DeltaTime = s_UpdateTimer.ElapsedSeconds();
		s_UpdateTimer.Reset();

		CommandHandler::Update();

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
			if (s_Settings.ImGuiEnabled)
				s_ImGuiHelper->Render();
			s_Renderer->End();

			s_Window->Update();

			s_AudioEngine->Update();
		}

		if (s_Settings.MaxFPS > 0) {
			float updateTime = s_UpdateTimer.ElapsedSeconds();
			float timeLeft = (1.f / s_Settings.MaxFPS) - updateTime;
			if (timeLeft > 0.f) {
				SE_PROFILE_SCOPE("MaxFpsCap");
				Time::MicroSleep((uint64)(floor(timeLeft * 1000 * 1000))); // Wait for microseconds
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

	void Engine::ForceShutdown() {
		Shutdown();

		std::exit(0);
	}

	void Engine::ForceShutdown(bool error, std::string_view msg) {
		if (error)
			Platform::MessagePrompts::Error("Forced Engine Shutdown", msg);
		else
			Platform::MessagePrompts::Info("Forced Engine Shutdown", msg);

		ForceShutdown();
	}
}