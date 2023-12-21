#pragma once

#include "CoreInclude.hpp"
#include "Application.hpp"
#include "ProjectSystem/Project.hpp"
#include "Serialization/YAMLSerializer.hpp"

namespace Sphynx {
	struct EngineSettings {
		bool Headless = false;
		bool ImGuiEnabled = false;
		std::string WindowName;
		bool Fullscreen = false;
		bool VSync = false;

		void ParseArguments(int argc, const char** argv) {
			for (int i = 0; i < argc; i++) {
				std::string_view arg(argv[i]);

				if (arg == "-headless")
					Headless = true;

				if (arg == "-vsync")
					VSync = true;
			}
		}

		Result<bool, std::string> ParseConfigFile(const std::filesystem::path& filepath) {
			std::string filepathStr = filepath.string();

			auto result = YAMLSerializer::LoadFile(filepathStr);
			if (result.is_error())
				return Error<bool>("Failed to parse config file: {}", result.get_error());

			YAML::Node& data = *result;
			VSync = data["VSync"].as<bool>();

			return true;
		}
	};
}