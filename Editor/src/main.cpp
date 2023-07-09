#include "pch.hpp"
#include "Core/Engine.hpp"

int main(int argc, const char** argv) {
	Sphynx::ConsoleArguments arguments(argc, argv);
	// get project dir

	Sphynx::EngineInitInfo initInfo {
		arguments
	};
	Sphynx::Engine::Init(initInfo);
	
	while (!Sphynx::Engine::ShouldClose()) {
		Sphynx::Engine::Update();
	}

	Sphynx::Engine::Shutdown();

	return 0;
}