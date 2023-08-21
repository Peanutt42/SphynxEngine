#pragma once

#include <string>
#include <vector>
#include "Core/EngineApi.hpp"

namespace Sphynx {	
	class SE_API CrashHandler {
	public:
		static void Init();

		static void StartCrashReporter();

		static void OnCrash(const std::string reason = "Engine decision (SE_ASSERT/SE_FATAL)",
							bool msgBox = false);

	private:
		inline static bool s_Initialized = false;
	};
}