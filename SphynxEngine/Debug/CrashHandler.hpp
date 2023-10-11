#pragma once

#include "pch.hpp"

namespace Sphynx {	
	class SE_API CrashHandler {
	public:
		static void Init();

		static void StartCrashReporter();

		static void OnCrash(const std::string reason, bool msgBox = false);

	private:
		inline static bool s_Initialized = false;
	};
}