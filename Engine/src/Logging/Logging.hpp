#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"
#include "Debug/CrashHandler.hpp"

namespace Sphynx {
	class SE_API Logging {
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
		constexpr static const char* CategoryToString(Logging::Category category);

		enum class Verbosity {
			Trace,
			Info,
			Warning,
			Error,
			Critical
		};

		static void Init();
		static void Shutdown();

		static void RegisterOnLogCallback(const std::function<void(Verbosity, Category, const std::string&)>& callback) {
			s_LogCallbacks.push_back(callback);
		}


		static void RawLog(Verbosity verbosity, Category category, const std::string& msg);

		template<typename... Args>
		static std::string Log(Verbosity verbosity, Category category, std::format_string<Args...> msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, category, formatted);
			return formatted;
		}

		template<typename... Args>
		static std::string Log(Verbosity verbosity, std::format_string<Args...> msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, Category::General, formatted);
			return formatted;
		}

		template<typename... Args>
		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, Category category, const std::format_string<Args...>& msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			std::string finalMsg = std::format("Assertion failed in {}\n{}: {}", file, expression, formatted);
			RawLog(verbosity, category, finalMsg);
			return finalMsg;
		}

		template<typename... Args>
		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, std::format_string<Args...> msg, Args&&... args) {
			std::string formatted = std::format(msg, std::forward<Args>(args)...);
			std::string finalMsg = std::format("Assertion failed in {}\n{}: {}", file, expression, formatted);
			RawLog(verbosity, Category::General, finalMsg);
			return finalMsg;
		}

		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, const char* assertMsg) {
			std::string finalMsg = std::format("Assertion failed in {}\n{}: {}", file, expression, assertMsg);
			RawLog(verbosity, Category::General, finalMsg);
			return finalMsg;
		}

		static const std::vector<std::string>& GetErrorList() { return s_ErrorMessageStack; }

	private:
		inline static bool s_Initialized = false;
		inline static std::mutex s_Mutex;
		
		inline static std::vector<std::string> s_ErrorMessageStack;
	
		inline static Verbosity s_Verbosity = Verbosity::Info;

		inline static std::vector<std::function<void(Verbosity, Category, const std::string&)>> s_LogCallbacks;

		inline static std::ofstream s_LogFile;
	};

	constexpr const char* Logging::CategoryToString(Logging::Category category) {
		switch (category) {
		default:
		case Logging::Category::General:		return "[General]        ";
		case Logging::Category::Game:			return "[Game]           ";
		case Logging::Category::Editor:			return "[Editor]         ";
		case Logging::Category::Runtime:		return "[Runtime]        ";
		case Logging::Category::Audio:			return "[Audio]          ";
		case Logging::Category::AssetManagment:	return "[Assets]         ";
		case Logging::Category::Serialization:	return "[Serialization]  ";
		case Logging::Category::Memory:			return "[Memory]         ";
		case Logging::Category::Networking:		return "[Networking]     ";
		case Logging::Category::Scripting:		return "[Scripting]      ";
		case Logging::Category::ECS:			return "[ECS]            ";
		case Logging::Category::Physics:		return "[Physics]        ";
		case Logging::Category::Rendering:		return "[Rendering]      ";
		case Logging::Category::UI:				return "[UI]             ";
		case Logging::Category::Building:		return "[Building]       ";
		}
	}


#if defined(DEBUG) || defined(RELEASE)

#define SE_TRACE(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Trace, __VA_ARGS__)
#define SE_INFO(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Info, __VA_ARGS__)
#define SE_WARN(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Warning, __VA_ARGS__)
#define SE_ERR(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Error, __VA_ARGS__)
#define SE_FATAL(...) \
{																																		\
	std::string formatted = Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);									\
	if (Sphynx::Platform::IsDebuggerAttached())																							\
		__debugbreak();																													\
	else																																\
		Sphynx::CrashHandler::OnCrash("SE_FATAL: " + formatted);																	\
}
#define SE_ASSERT(result, ...) {	\
	if (!(result)) {																													\
		std::string formatted = Sphynx::Logging::AssertLog(#result, __FILE__, Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);		\
		if (Sphynx::Platform::IsDebuggerAttached())																						\
			__debugbreak();																												\
		else																															\
			Sphynx::CrashHandler::OnCrash("SE_ASSERT: " + formatted);																\
	}																																	\
}

#else

#define SE_TRACE(...)			{}
#define SE_INFO(...)			{}
#define SE_WARN(...)			{}
#define SE_ERR(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Error, __VA_ARGS__)
#define SE_FATAL(...)			{ std::string formatted = Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Critical, __VA_ARGS__); Sphynx::CrashHandler::OnCrash("SE_FATAL: " + formatted); }
#define SE_ASSERT(result, ...) {	\
	if (!(result)) {																													\
		std::string formatted = Sphynx::Logging::AssertLog(#result, __FILE__, Sphynx::Logging::Verbosity::Critical, __VA_ARGS__);		\
		Sphynx::CrashHandler::OnCrash("SE_ASSERT: " + formatted);																		\
	}																																	\
}

#endif
}