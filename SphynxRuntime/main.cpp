#include "pch.hpp"
#include "Core/EntryPoint.hpp"
#include "Core/Application.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Rendering/Renderer.hpp"
#include "Physics/PhysicEngine.hpp"

namespace Sphynx {
	class RuntimeApplication : public Application {
	public:


		virtual void OnCreate() override {
			m_Scene = std::make_unique<Scene>("Scene");

			SceneSerializer::Deserialize(Engine::GetProject().StartSceneFilepath, *m_Scene)
				.expect("Failed to open start scene");

			if (Rendering::Renderer::IsInitialized())
				Rendering::Renderer::SetDrawSceneTextureEnabled(true);
		}

		virtual void OnDestroy() override {

		}

		virtual void Update() override {
			Physics::PhysicEngine::Update(*m_Scene);

            if (Rendering::Renderer::IsInitialized())
				Rendering::Renderer::SubmitScene(*m_Scene, Rendering::Camera{});
		}

		virtual void DrawUI() override {

		}

	private:
		std::unique_ptr<Scene> m_Scene;
	};
}

int GuardedMain(int argc, const char** argv) {
	// Get project filepath
	std::filesystem::path projectFilepath;
	if (argc >= 2) {
		projectFilepath = argv[1];
		projectFilepath = std::filesystem::absolute(projectFilepath);
	}
	else {
		std::cout << "Usage: [projectFilepath]\n";
		return 1;
	}

	Sphynx::Platform::SetWorkingDirToExe();
	
	Sphynx::Project project(projectFilepath);

	if (project.EngineVersion != Sphynx::Engine::EngineVersion) {
		std::string error = fmt::format("The project's version ({}) is a diffrent version than this Engine version ({})", project.EngineVersion.ToString(), Sphynx::Engine::EngineVersion.ToString());
		Sphynx::Platform::MessagePrompts::Error("Project's engine version", error);
		return 0;
	}

	Sphynx::EngineSettings settings;
	settings.ParseArguments(argc, argv);
	settings.WindowName = "Sphynx Engine Runtime - " + project.Name;
	settings.Fullscreen = true;
	settings.ImGuiEnabled = false;

	Sphynx::RuntimeApplication application;
	Sphynx::Engine::Init(settings, project, application);
	
	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}