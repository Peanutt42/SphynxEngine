#pragma once

#include <string>
#include <vector>
#include "Core/EngineApi.hpp"
#include "StackTrace.hpp"

namespace Sphynx {
	class SE_API CrashHandler {
	public:
		static void Init();

		// 'cutSetup' cuts everything until the main function
		static StackTrace MakeStackTrace(bool cutSetup);

		// signal = -1 -> singal gets ignored
		static void OnProcessCrashed(int signal = -1);
	};
}