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
			Engine::Renderer().SetDrawSceneTextureEnabled(true);

			m_Scene = std::make_unique<Scene>("Scene");

			std::string errorMsg;
			bool successful = SceneSerializer::Deserialize(Engine::GetProject()->StartSceneFilepath, *m_Scene, errorMsg);
			SE_ASSERT(successful, "Failed to open scene: {}", errorMsg);
		}

		virtual void OnDestroy() override {

		}

		virtual void Update() override {
			Engine::Physics().Update(*m_Scene);

			for (const auto& system : Engine::Scripting().GetSystems())
				system.Update((void*)m_Scene.get());

			Engine::Renderer().SubmitScene(*m_Scene, Rendering::Camera{});
			// TODO: Draw Quad with the scene tex
		}

		virtual void DrawUI() override {

		}

	private:
		std::unique_ptr<Scene> m_Scene;
	};
}

int GuardedMain(int argc, const char** argv) {
	Sphynx::Platform::SetWorkingDirToExe();
	// SphynxEngine/bin/$Platform/$Config/Editor/Editor.exe
	Sphynx::Platform::SetWorkingDirToParentFolder(4);

	// Get project filepath
	std::filesystem::path projectFilepath;
	if (argc >= 2)
		projectFilepath = argv[1];
	
	if (!std::filesystem::exists(projectFilepath))
		projectFilepath = Sphynx::Platform::FileDialogs::OpenFile("Sphynx Engine Project", "*.seproj");
	if (!std::filesystem::exists(projectFilepath))
		return 0;


	// Load project
	std::shared_ptr<Sphynx::Project> project = std::make_shared<Sphynx::Project>(projectFilepath);
	if (project->EngineVersion != Sphynx::Engine::Version) {
		Sphynx::Platform::MessagePrompts::Error("Project's engine version", std::format("The project's version ({}) is a diffrent version than this Engine version ({})", project->EngineVersion, Sphynx::Engine::Version));
		return 0;
	}

	// Create application
	std::shared_ptr<Sphynx::RuntimeApplication> application = std::make_unique<Sphynx::RuntimeApplication>();

	

	Sphynx::EngineSettings engineSettings;
	engineSettings.ParseConfigFile(project->EngineConfigFilepath);
	engineSettings.ParseArguments(argc, argv);
	engineSettings.WindowName = "Sphynx Engine Runtime";
	engineSettings.Fullscreen = true;
	engineSettings.ImGuiEnabled = false;

	Sphynx::EngineInitInfo initInfo {
		.Settings = engineSettings,
		.Project = project,
		.Application = application
	};

	Sphynx::Engine::Init(initInfo);
	
	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}