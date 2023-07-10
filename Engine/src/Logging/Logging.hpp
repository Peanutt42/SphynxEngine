#pragma once

#include "std.hpp"
#include "Debug/CrashHandler.hpp"

namespace Sphynx {
	class Logging {
	public:
		enum Category {
			General,
			Game,
			Editor,
			Runtime,
			Audio,
			AssetManagment,
			Serialization,
			Memory,
			Networking,
			Scripting,
			ECS,
			Physics,
			Rendering,
			UI,
			Building
		};

		enum class Verbosity {
			Trace,
			Info,
			Warning,
			Error,
			Critical
		};

		static void Init();
		static void Shutdown();


		static void RawLog(Verbosity verbosity, Category category, const std::string& msg);

		template<typename... Args>
		static void Log(Verbosity verbosity, Category category, std::format_string<Args...> msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, category, formatted);
		}

		template<typename... Args>
		static void Log(Verbosity verbosity, std::format_string<Args...> msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, Category::General, formatted);
		}

		static void Log(Verbosity verbosity) {
			RawLog(verbosity, Category::General, "Empty log function (probably a SE_ASSERT(false);)");
		}

		static const std::vector<std::string>& GetErrorList() { return s_ErrorMessageStack; }

	private:
		inline static bool s_Initialized = false;
		inline static std::mutex s_Mutex;
		
		inline static std::vector<std::string> s_ErrorMessageStack;
	
		inline static Verbosity s_Verbosity = Verbosity::Info;

		inline static std::ofstream s_LogFile;
	};


#if defined(DEBUG) || defined(RELEASE)

#define SE_TRACE(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Trace, __VA_ARGS__)
#define SE_INFO(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Info, __VA_ARGS__)
#define SE_WARN(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Warning, __VA_ARGS__)
#define SE_ERR(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Error, __VA_ARGS__)
#define SE_FATAL(...) \
{																																		\
	Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);												\
	if (Sphynx::Platform::IsDebuggerAttached())																							\
		__debugbreak();																													\
	else																																\
		Sphynx::CrashHandler::OnProcessCrashed();																						\
}
#define SE_ASSERT(result, ...) { if (!(result)) SE_FATAL(__VA_ARGS__) }

#else

#define SE_TRACE(category, msg, ...)			{}
#define SE_INFO(category, msg, ...)				{}
#define SE_WARN(category, msg, ...)				{}
#define SE_ERR(category, msg, ...)				{}
#define SE_FATAL(category, msg, ...)			Sphynx::CrashHandler::OnProcessCrashed()
#define SE_ASSERT(result, category, msg, ...)	{ if (!(result)) SE_FATAL(category, msg, __VA_ARGS__); }

#endif
}