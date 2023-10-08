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

	Sphynx::Project project(projectFilepath);
	if (project.EngineVersion != Sphynx::Engine::Version) {
		std::string error = std::format("The project's version ({}) is a diffrent version than this Engine version ({})", project.EngineVersion.ToString(), Sphynx::Engine::Version.ToString());
		Sphynx::Platform::MessagePrompts::Error("Project's engine version missmatch", error);
		return 0;
	}

	Sphynx::EngineSettings settings;
	settings.ParseConfigFile(project.EngineConfigFilepath);
	settings.ParseArguments(argc, argv);
	settings.Headless = false; // forced
	settings.WindowName = "Sphynx Engine Editor - " + project.Name;
	settings.Fullscreen = false;
	settings.ImGuiEnabled = true;

	Sphynx::Editor::EditorApplication application;
	Sphynx::Engine::Init(settings, project, application);

	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}