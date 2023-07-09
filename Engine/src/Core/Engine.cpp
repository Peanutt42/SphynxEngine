#include "pch.hpp"
#include "Engine.hpp"

namespace Sphynx {
	void Engine::Init(const EngineInitInfo& info) {
		s_Settings.ParseConsoleArguments(info.Arguments);

		Logging::Init();

		SE_INFO(Logging::General, "=== SPHYNX ENGINE INIT ===");
	}

	void Engine::Shutdown() {
		SE_INFO(Logging::General, "=== SPHYNX ENGINE SHUTDOWN ===");

		Logging::Shutdown();
	}

	void Engine::Update() {

	}
}