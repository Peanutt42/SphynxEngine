#include "pch.hpp"
#include "Core/Engine.hpp"

int main(int argc, const char** argv) {
	SE_ASSERT(argc >= 1, Sphynx::Logging::General, "There should be at least 1 argument being the .exe filepath");

	// Set working directory to the rootDir
	std::filesystem::path exeFilepath = argv[0];
									// SphynxEngine/bin/$Platform/$Config/Editor/Editor.exe
	std::filesystem::current_path(exeFilepath.parent_path().parent_path().parent_path().parent_path().parent_path());

	// Get project filepath
	std::filesystem::path projectFilepath;
	if (argc >= 2)
		projectFilepath = argv[1];
	
	if (!std::filesystem::exists(projectFilepath))
		projectFilepath = Sphynx::Platform::FileDialogs::OpenFile("Sphynx Engine Project", "*.seproj");
	
	SE_ASSERT(std::filesystem::exists(projectFilepath), Sphynx::Logging::General, "Provided project file '{}' doesn't exist", projectFilepath.string());
	
	// Load project
	std::shared_ptr<Sphynx::Project> project = std::make_shared<Sphynx::Project>(projectFilepath);
	SE_ASSERT(project->EngineVersion == Sphynx::Engine::Version, Sphynx::Logging::General, "The project's version ({}) is a diffrent version that this Engine version ({})", project->EngineVersion.to_string(), Sphynx::Engine::Version.to_string());

	Sphynx::ConsoleArguments arguments(argc, argv);

	Sphynx::EngineSettings engineSettings;
	engineSettings.ParseConfigFile(project->EngineConfigFilepath);
	engineSettings.ParseArguments(arguments);
	engineSettings.Headless = false; // forced
	engineSettings.WindowName = "Sphynx Engine Editor";
	engineSettings.Fullscreen = false;

	Sphynx::EngineInitInfo initInfo {
		.Settings = engineSettings,
		.Project = project
	};

	Sphynx::Engine::Init(initInfo);
	
	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}