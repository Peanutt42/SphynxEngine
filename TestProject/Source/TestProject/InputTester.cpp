#include "Core/CoreInclude.hpp"

using namespace Sphynx;

extern "C" {
	__declspec(dllexport) void TestInput() {
		if (Input::IsKeyPressed(KeyCode::Enter))
			SE_INFO(Logging::Game, "Enter pressed");
	}
}