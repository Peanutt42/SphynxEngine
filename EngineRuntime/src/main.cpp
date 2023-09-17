#include "pch.hpp"
#include "Core/EntryPoint.hpp"
#include "Core/Application.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneSerializer.hpp"
#include "Scripting/ScriptingEngine.hpp"
#include "Rendering/Renderer.hpp"
#include "Physics/PhysicEngine.hpp"

namespace Sphynx {
	class RuntimeApplication : public Application {
	public:


		virtual void OnCreate() override {
			if (!Engine::GetSettings().Headless)
                Engine::Renderer().SetDrawSceneTextureEnabled(true);

			m_Scene = std::make_unique<Scene>("Scene");

			SceneSerializer::Deserialize(Engine::GetProject().StartSceneFilepath, *m_Scene)
				.expect("Failed to open start scene");
		}

		virtual void OnDestroy() override {

		}

		virtual void Update() override {
			Engine::Physics().Update(*m_Scene);

			for (const auto& system : Engine::Scripting().GetSystems())
				system.Update(*m_Scene);

            if (!Engine::GetSettings().Headless) {
                Engine::Renderer().SubmitScene(*m_Scene, Rendering::Camera{});
                // TODO: Draw Quad with the scene tex
            }
		}

		virtual void DrawUI() override {

		}

	private:
		std::unique_ptr<Scene> m_Scene;
	};
}

int GuardedMain(int argc, const char** argv) {
	Sphynx::Platform::SetWorkingDirToExe();

	// Get project filepath
	std::filesystem::path projectFilepath;
	if (argc >= 2)
		projectFilepath = argv[1];
	
	if (!std::filesystem::exists(projectFilepath))
		projectFilepath = Sphynx::Platform::FileDialogs::OpenFile("Sphynx Engine Project", "*.seproj");
	if (!std::filesystem::exists(projectFilepath))
		return 0;

	Sphynx::Project project(projectFilepath);

	if (project.EngineVersion != Sphynx::Engine::Version) {
		std::string error = std::format("The project's version ({}) is a diffrent version than this Engine version ({})", project.EngineVersion.ToString(), Sphynx::Engine::Version.ToString());
		Sphynx::Platform::MessagePrompts::Error("Project's engine version", error);
		return 0;
	}

	Sphynx::EngineSettings settings;
	settings.ParseConfigFile(project.EngineConfigFilepath);
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