#pragma once

#include <string>
#include <vector>
#include "StackTrace.hpp"

namespace Sphynx {
	class CrashHandler {
	public:
		static void Init();

		// 'cutSetup' cuts everything until the main function
		static StackTrace MakeStackTrace(bool cutSetup);

		static void OnProcessCrashed();
	};
}