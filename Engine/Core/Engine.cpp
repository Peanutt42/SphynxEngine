#include "pch.hpp"
#include "Engine.hpp"
#include "CommandHandler.hpp"

#include "Audio/AudioEngine.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/Window.hpp"
#include "Physics/PhysicEngine.hpp"
#include "Scripting/ScriptingEngine.hpp"
#include "UI/VulkanImGui.hpp"

namespace Sphynx {
	SE_API std::atomic_bool s_Quit = false;

	SE_API EngineSettings s_Settings;

	SE_API float s_DeltaTime = 0.f;
	SE_API Timer s_UpdateTimer;

	SE_API Project* s_Project = nullptr;

	SE_API Application* s_Application = nullptr;

	SE_API Rendering::Window* s_Window = nullptr;

	void Engine::Init(const EngineSettings& settings, Project& project, Application& application) {
		CrashHandler::Init();
		CrashHandler::StartCrashReporter();

		s_Settings = settings;

		Logging::Init();

		SE_INFO("=== SPHYNX ENGINE INIT ===");

		s_Project = &project;

		s_Application = &application;

		Physics::PhysicEngine::Init();

		SE_ASSERT(Scripting::ScriptingEngine::Init(), "Failed to initialize Scripting Engine");

		if (s_Settings.Headless) {
			ConsoleInput::Init();
			ConsoleInput::SetInputCallback(CommandHandler::QueueCommand);
		}
		else {
			SE_ASSERT(Audio::AudioEngine::Init(), "Failed to initialize Audio Engine");

			s_Window = new Rendering::Window(s_Settings.WindowName, true, s_Settings.Fullscreen, s_Settings.CustomWindowControls);

			Input::Init(s_Window->GetGLFWHandle());

			SE_ASSERT(Rendering::Renderer::Init(*s_Window, &Update), "Failed to initialize the Renderer");

			if (s_Settings.ImGuiEnabled)
				UI::VulkanImGui::Init();
		}

		s_Application->OnCreate();

		s_UpdateTimer.Reset();
	}

	void Engine::Shutdown() {
		s_Application->OnDestroy();

		if (Rendering::Renderer::IsInitialized())
			Rendering::Renderer::WaitBeforeClose();

		Scripting::ScriptingEngine::Shutdown();

		Physics::PhysicEngine::Shutdown();

		if (ConsoleInput::IsInitialized())
			ConsoleInput::Shutdown();
		
		if (s_Settings.ImGuiEnabled)
			UI::VulkanImGui::Shutdown();

		if (Rendering::Renderer::IsInitialized())
			Rendering::Renderer::Shutdown();
		
		if (s_Window)
			delete s_Window;
		
		if (Audio::AudioEngine::IsInitialized())
			Audio::AudioEngine::Shutdown();

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

		Scripting::ScriptingEngine::Update();

		s_Application->Update();

		if (Rendering::Renderer::IsInitialized()) {
			if (s_Settings.ImGuiEnabled) {
				UI::VulkanImGui::Begin();
				s_Application->DrawUI();
				UI::VulkanImGui::End();
			}

			Rendering::Renderer::Begin();
			if (s_Settings.ImGuiEnabled)
				UI::VulkanImGui::Render();
			Rendering::Renderer::End();
		}
		
		if (s_Window)
			s_Window->Update();

		if (Audio::AudioEngine::IsInitialized())
			Audio::AudioEngine::Update();

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

	float Engine::DeltaTime() { return s_DeltaTime; }

	EngineSettings& Engine::GetSettings() { return s_Settings; }

	Project& Engine::GetProject() { return *s_Project; }

	void Engine::CloseNextFrame() { s_Quit.store(true); }

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