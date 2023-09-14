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
	std::shared_ptr<Sphynx::Project> project = std::make_shared<Sphynx::Project>("../../../../TestProject/TestProject.seproj");

	Sphynx::EngineSettings engineSettings {
		.Headless = true,
		.ImGuiEnabled = false,
		.WindowName = "Headless Engine Lifecycle Test",
		.Fullscreen = false,
	};

	Sphynx::EngineInitInfo initInfo{
		.Settings = engineSettings,
		.Project = project,
		.Application = std::make_shared<NullApplication>(),
	};

	Sphynx::Engine::Init(initInfo);
	Sphynx::Engine::Update();
	Sphynx::Engine::Shutdown();
}