#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::Audio {
	class SE_API AudioEngine {
	public:
		static bool Init();
		static void Shutdown();

		static void Update();

		static bool IsInitialized();
	};
}