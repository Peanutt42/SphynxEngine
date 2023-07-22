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

	namespace Scripting {
		class ScriptingEngine;
	}

	namespace UI {
		class VulkanImGuiHelper;
	}

	class SE_API Engine {
	public:
		static void Init(const EngineInitInfo& initInfo);
		static void Shutdown();

		static void Update();

		static void CloseNextFrame() { s_Quit.store(true); }

		static bool ShouldClose();

		static float DeltaTime() { return s_DeltaTime; }

		static EngineSettings& GetSettings() { return s_Settings; }

		static std::shared_ptr<Project> GetProject() { return s_Project; }

		static Scripting::ScriptingEngine& Scripting() { return *s_ScriptingEngine; }
		static Rendering::Renderer& Renderer() { return *s_Renderer; }

		static UI::VulkanImGuiHelper& ImGuiHelper() { return *s_ImGuiHelper; }

		constexpr static Version Version { 0, 0, 1 };

	private:
		inline static std::atomic_bool s_Quit = false;

		inline static EngineSettings s_Settings;

		inline static float s_DeltaTime = 0.f;
		inline static Timer s_UpdateTimer;

		inline static std::shared_ptr<Project> s_Project;

		inline static std::shared_ptr<Application> s_Application;

		inline static Rendering::Window* s_Window = nullptr;
		inline static Rendering::Renderer* s_Renderer = nullptr;
		inline static Scripting::ScriptingEngine* s_ScriptingEngine = nullptr;
		inline static UI::VulkanImGuiHelper* s_ImGuiHelper = nullptr;
	};
}