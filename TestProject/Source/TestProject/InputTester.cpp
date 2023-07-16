#include "Core/CoreInclude.hpp"

extern "C" {
	__declspec(dllexport) void TestInput() {
		SE_INFO(Sphynx::Logging::Game, "Enter pressed");
	}
}