#include "pch.hpp"
#include "Core/EntryPoint.hpp"
#include "EditorApplication.hpp"

#include <misc/cpp/imgui_stdlib.cpp>

int GuardedMain(int argc, const char** argv) {
	Sphynx::Platform::SetWorkingDirToExe();
	// SphynxEngine/bin/$Platform/$Config/Editor/Editor.exe
	Sphynx::Platform::SetWorkingDirToParentFolder(4);

	// Get project filepath
	std::filesystem::path projectFilepath;
	if (argc >= 2)
		projectFilepath = argv[1];
	
	while (true) {
		projectFilepath = Sphynx::Platform::FileDialogs::OpenFile("Sphynx Engine Project", "*.seproj");
		if (std::filesystem::exists(projectFilepath))
			break;

		bool answer = Sphynx::Platform::MessagePrompts::YesNo("Open Project", "Do you want to try again?");
		if (answer == false)
			return 0;
	}
	
	// Load project
	std::shared_ptr<Sphynx::Project> project = std::make_shared<Sphynx::Project>(projectFilepath);
	if (project->EngineVersion != Sphynx::Engine::Version) {
		Sphynx::Platform::MessagePrompts::Error("Project's engine version", std::format("The project's version ({}) is a diffrent version than this Engine version ({})", project->EngineVersion, Sphynx::Engine::Version));
		return 0;
	}

	Sphynx::ConsoleArguments arguments(argc, argv);

	Sphynx::EngineSettings engineSettings;
	engineSettings.ParseConfigFile(project->EngineConfigFilepath);
	engineSettings.ParseArguments(arguments);
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