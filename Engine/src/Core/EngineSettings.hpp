#pragma once

#include "CoreInclude.hpp"
#include "Application.hpp"
#include "ProjectSystem/Project.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	struct SE_API EngineSettings {
		bool Headless = false;
		bool ImGuiEnabled = false;
		std::string WindowName;
		bool Fullscreen = false;
		bool CustomWindowControls = true;
		int MaxFPS = 0;

		void ParseArguments(int argc, const char** argv) {
			for (int i = 1; i < argc; i++) {
				std::string_view arg(argv[i]);

				if (arg == "-headless")
					Headless = true;

				if (arg.starts_with("-maxfps=")) {
					if (auto result = StringToNumber<int>(arg.substr(std::size("-maxfps=") - 1)))
						MaxFPS = *result;
				}
			}
		}

		void ParseConfigFile(const std::filesystem::path& filepath) {
			std::string filepathStr = filepath.string();

			YAML::Node data;
			if (YAMLSerializer::LoadFile(filepathStr, data)) {
				MaxFPS = data["MaxFPS"].as<int>();
			}
		}
	};

	struct EngineInitInfo {
		EngineSettings Settings;
		std::shared_ptr<Project> Project;
		std::shared_ptr<Application> Application;
	};	
}