#pragma once

#include "CoreInclude.hpp"
#include "ConsoleArguments.hpp"
#include "EngineSettings.hpp"

namespace Sphynx {
	struct EngineInitInfo {
		ConsoleArguments Arguments;
	};

	class Engine {
	public:
		static void Init(const EngineInitInfo& info);
		static void Shutdown();

		static void Update();

		static void CloseNextFrame() { s_Quit.store(true); }

		static bool ShouldClose() { return s_Quit.load(); }

	private:
		inline static std::atomic_bool s_Quit = false;

		inline static EngineSettings s_Settings;
	};
}