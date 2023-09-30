#pragma once

#include <string>
#include <vector>

namespace Sphynx {	
	class CrashHandler {
	public:
		static void Init();

		static void StartCrashReporter();

		static void OnCrash(const std::string reason, bool msgBox = false);

	private:
		inline static bool s_Initialized = false;
	};
}