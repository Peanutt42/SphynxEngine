#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"
#include "ProjectSystem/Project.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	struct EngineSettings {
		bool Headless = false;
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
	};	
}