#include "pch.hpp"
#include "Core/Engine.hpp"

class NullApplication : public Sphynx::Application {
public:
	virtual void OnCreate() override {}
	virtual void OnDestroy() override {}

	virtual void Update() override {}
	virtual void DrawUI() override {}
};

TEST(Engine, HeadlessEngineLifecyle) {
	Sphynx::Platform::SetWorkingDirToExe();
	
	Sphynx::EngineSettings engineSettings {
		.Headless = true,
		.ImGuiEnabled = false,
		.WindowName = "Headless Engine Lifecycle Test",
		.Fullscreen = false,
	};

	Sphynx::Project project("../../../TestProject/TestProject.seproj");
	NullApplication application;

	Sphynx::Engine::Init(engineSettings, project, application);
	Sphynx::Engine::Update();
	Sphynx::Engine::Shutdown();
}