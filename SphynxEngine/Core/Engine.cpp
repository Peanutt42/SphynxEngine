#include "pch.hpp"
#include "Engine.hpp"
#include "CommandHandler.hpp"
#include "Time.hpp"

#include "Audio/AudioEngine.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/Window.hpp"
#include "Scripting/ScriptingEngine.hpp"
#include "Physics/PhysicEngine.hpp"
#include "Input/Input.hpp"
#include "Profiling/Profiling.hpp"
#include "UI/UI.hpp"

namespace Sphynx {
	std::atomic_bool s_Quit = false;

	EngineSettings s_Settings;

	float s_DeltaTime = 0.f;
	Timer s_UpdateTimer;

	Project* s_Project = nullptr;

	Application* s_Application = nullptr;

	Rendering::Window* s_Window = nullptr;

	void Engine::Init(const EngineSettings& settings, Project& project, Application& application) {
		CrashHandler::Init();
		CrashHandler::StartCrashReporter();

		Timer initTimer;

		s_Settings = settings;

		Logging::Init();

		SE_INFO("=== SPHYNX ENGINE INIT ===");

		s_Project = &project;

		s_Application = &application;

		Physics::PhysicEngine::Init();

		Scripting::ScriptingEngine::Init();

		if (s_Settings.Headless) {
			ConsoleInput::Init();
			ConsoleInput::SetInputCallback(CommandHandler::QueueCommand);
		}
		else {
			SE_ASSERT(Audio::AudioEngine::Init(), "Failed to initialize Audio Engine");

			s_Window = new Rendering::Window(s_Settings.WindowName, false, s_Settings.Fullscreen);

			Input::Init(s_Window->GetGLFWHandle());

			SE_ASSERT(Rendering::Renderer::Init(*s_Window, &Update, s_Settings.VSync), "Failed to initialize the Renderer");

			if (s_Settings.ImGuiEnabled)
				UI::Init(*s_Window);
		}

		s_Application->OnCreate();

		s_UpdateTimer.Reset();

		SE_INFO("Init took: {}s", initTimer.ElapsedSeconds());
	}

	void Engine::Shutdown() {
		Timer shutdownTimer;

		s_Application->OnDestroy();

		Scripting::ScriptingEngine::Shutdown();

		Physics::PhysicEngine::Shutdown();

		if (ConsoleInput::IsInitialized())
			ConsoleInput::Shutdown();

		if (s_Settings.ImGuiEnabled)
			UI::Shutdown();
		if (Rendering::Renderer::IsInitialized())
			Rendering::Renderer::Shutdown();

		if (s_Window)
			delete s_Window;

		if (Audio::AudioEngine::IsInitialized())
			Audio::AudioEngine::Shutdown();

		SE_INFO("Shutdown took: {}s", shutdownTimer.ElapsedSeconds());

		SE_INFO("=== SPHYNX ENGINE SHUTDOWN ===");

		Logging::Shutdown();
	}

	void Engine::Update() {
		SE_PROFILE_FUNCTION();
		s_DeltaTime = s_UpdateTimer.ElapsedSeconds();
		s_UpdateTimer.Reset();

		CommandHandler::Update();

		if (s_Window)
			Input::Update();

		s_Application->Update();

		Scripting::ScriptingEngine::Update();

		if (Rendering::Renderer::IsInitialized()) {
			if (s_Settings.ImGuiEnabled) {
				UI::Begin();
				s_Application->DrawUI();
				UI::End();
			}

			Rendering::Renderer::Update();
			if (s_Settings.ImGuiEnabled)
				UI::Render();
		}
				
		if (s_Window)
			s_Window->Update();

		if (Audio::AudioEngine::IsInitialized())
			Audio::AudioEngine::Update();

		SE_PROFILE_FRAME_END("Main Thread");
	}

	bool Engine::ShouldClose() {
		if (s_Quit.load())
			return true;
		if (!s_Settings.Headless && s_Window && s_Window->ShouldClose())
			return true;
		return false;
	}

	float Engine::DeltaTime() { return s_DeltaTime; }

	EngineSettings& Engine::GetSettings() { return s_Settings; }

	Project& Engine::GetProject() { return *s_Project; }

	void Engine::CloseNextFrame() { s_Quit.store(true); }

	void Engine::ForceShutdown() {
		Logging::Shutdown();

		std::exit(0);
	}

	void Engine::ForceShutdown(std::string_view msg) {
		Platform::MessagePrompts::Error("Forced Engine Shutdown", msg);
		
		ForceShutdown();
	}
}