#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::Audio {
	class AudioEngine {
	public:
		static bool Init();
		static void Shutdown();

		static void Update();

		static bool IsInitialized();
	};
}