#include "pch.hpp"
#include "Core/EntryPoint.hpp"
#include "EditorApplication.hpp"

#include <misc/cpp/imgui_stdlib.cpp>

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

	
	// Load project
	std::shared_ptr<Sphynx::Project> project = std::make_shared<Sphynx::Project>(projectFilepath);
	if (project->EngineVersion != Sphynx::Engine::Version) {
		Sphynx::Platform::MessagePrompts::Error("Project's engine version", std::format("The project's version ({}) is a diffrent version than this Engine version ({})", project->EngineVersion, Sphynx::Engine::Version));
		return 0;
	}


	Sphynx::EngineSettings engineSettings;
	engineSettings.ParseConfigFile(project->EngineConfigFilepath);
	engineSettings.ParseArguments(argc, argv);
	engineSettings.Headless = false; // forced
	engineSettings.WindowName = "Sphynx Engine Editor";
	engineSettings.Fullscreen = false;
	engineSettings.ImGuiEnabled = true;

	Sphynx::EngineInitInfo initInfo {
		.Settings = engineSettings,
		.Project = project,
		.Application = std::make_shared<Sphynx::Editor::EditorApplication>()
	};

	Sphynx::Engine::Init(initInfo);

	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}