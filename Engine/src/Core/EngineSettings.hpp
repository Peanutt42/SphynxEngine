#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"

namespace Sphynx {
	struct EngineInitInfo {
		ConsoleArguments Arguments;
		std::string WindowName;
		bool Fullscreen = false;
	};

	struct EngineSettings {
		bool Headless = false;
		std::string WindowName;
		bool Fullscreen = false;
		float MaxFPS = -1.f;

		void FromInitInfo(const EngineInitInfo& initInfo) {
			if (initInfo.Arguments.HasArgument("-headless"))
				Headless = true;

			WindowName = initInfo.WindowName;
			Fullscreen = initInfo.Fullscreen;
		}
	};
}