#pragma once

#include <string>
#include <vector>
#include "Core/EngineApi.hpp"

namespace Sphynx {
	class SE_API CrashHandler {
	public:
		static void Init();

		// signal = -1 -> singal gets ignored
		static void OnProcessCrashed(int signal = -1);
	};
}