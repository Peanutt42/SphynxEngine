#pragma once

#include "pch.hpp"
#include "Logging/Logging.hpp"
#include "Platform/Platform.hpp"

namespace Sphynx {	
	class SE_API CrashHandler {
	public:
		static void Init();

		static void StartCrashReporter();

		static void OnCrash(const std::string& reason, bool msgBox = false);

	private:
		inline static bool s_Initialized = false;
	};
}

#if defined(DEBUG) || defined(DEVELOPMENT)
#define SE_FATAL(...) \
{																																		\
	if (Sphynx::Platform::IsDebuggerAttached())																							\
		DEBUGBREAK();																													\
	else {                                                                                                                              \
        std::string formatted = Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);                                \
        Sphynx::CrashHandler::OnCrash("SE_FATAL: " + formatted);                                                                        \
   }                                                                                                                                    \
}
#define SE_ASSERT(result, ...) {	\
	if (!(result)) {																													\
		if (Sphynx::Platform::IsDebuggerAttached())																						\
			DEBUGBREAK();																												\
		else {                         \
            std::string formatted = Sphynx::Logging::AssertLog(#result, __FILE__, Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);   \
            Sphynx::CrashHandler::OnCrash("SE_ASSERT: " + formatted);                                                                   \
        }                                                                                                                               \
        std::exit(1);	/* This will never be reached but suppresses warnings for not checking if the expr is 0 */					    \
	}																																	\
}
#else
#define SE_FATAL(...)			{ std::string formatted = Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Critical, __VA_ARGS__); Sphynx::CrashHandler::OnCrash("SE_FATAL: " + formatted); }
#define SE_ASSERT(result, ...) {	\
	if (!(result)) {																													\
		std::string formatted = Sphynx::Logging::AssertLog(#result, __FILE__, Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);		\
		Sphynx::CrashHandler::OnCrash("SE_ASSERT: " + formatted);																		\
	}																																	\
}
#endif