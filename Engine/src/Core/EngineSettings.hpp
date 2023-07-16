#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"
#include "Application.hpp"
#include "ProjectSystem/Project.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	struct SE_API EngineSettings {
		bool Headless = false;
		bool ImGuiEnabled = false;
		std::string WindowName;
		bool Fullscreen = false;
		float MaxFPS = -1.f;

		void ParseArguments(const ConsoleArguments& arguments) {
			if (arguments.HasArgument("-headless"))
				Headless = true;
		}

		void ParseConfigFile(const std::filesystem::path& filepath) {
			std::string filepathStr = filepath.string();

			YAML::Node data = YAML::LoadFile(filepathStr);
			MaxFPS = data["MaxFPS"].as<float>();
		}
	};

	struct EngineInitInfo {
		EngineSettings Settings;
		std::shared_ptr<Project> Project;
		std::shared_ptr<Application> Application;
	};	
}