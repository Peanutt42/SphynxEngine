#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"

namespace Sphynx {
	struct EngineSettings {
		bool Headless = false;

		void ParseConsoleArguments(const ConsoleArguments& arguments) {
			if (arguments.HasArgument("-headless"))
				Headless = true;
		}
	};
}