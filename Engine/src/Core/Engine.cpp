#include "pch.hpp"
#include "Engine.hpp"

namespace Sphynx {
	void Engine::Init(const EngineInitInfo& info) {
		s_Settings.ParseConsoleArguments(info.Arguments);
	}

	void Engine::Shutdown() {

	}

	void Engine::Update() {

	}
}