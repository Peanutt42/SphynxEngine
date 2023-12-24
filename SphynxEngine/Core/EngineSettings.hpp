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

		void ParseArguments(int argc, const char** argv) {
			for (int i = 0; i < argc; i++) {
				std::string_view arg(argv[i]);

				if (arg == "-headless")
					Headless = true;
			}
		}
	};
}