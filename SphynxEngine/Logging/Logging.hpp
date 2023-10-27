#pragma once

#include "std.hpp"
#include <fmt/core.h>

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
		static const char* CategoryToString(Category category);

		enum class Verbosity {
			Trace,
			Info,
			Warning,
			Error,
			Critical
		};
		static const char* VerbosityToString(Verbosity verbosity);

		static void Init();
		static void Shutdown();

		static void RegisterOnLogCallback(const std::function<void(Verbosity, Category, const std::string&)>& callback) {
			s_LogCallbacks.push_back(callback);
		}

		template<typename... Args>
		static std::string Log(Verbosity verbosity, Category category, fmt::format_string<Args...> msg, Args&&... args) {
			std::string formatted = fmt::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, category, formatted);
			return formatted;
		}

		template<typename... Args>
		static std::string Log(Verbosity verbosity, fmt::format_string<Args...> msg, Args&&... args) {
			std::string formatted = fmt::format(msg, std::forward<Args>(args)...);
			RawLog(verbosity, Category::General, formatted);
			return formatted;
		}

		template<typename... Args>
		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, Category category, const fmt::format_string<Args...>& msg, Args&&... args) {
			std::string formatted = fmt::format(msg, std::forward<Args>(args)...);
			std::string finalMsg = fmt::format("Assertion failed in {}\n{}: {}", file, expression, formatted);
			RawLog(verbosity, category, finalMsg);
			return finalMsg;
		}

		template<typename... Args>
		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, fmt::format_string<Args...> msg, Args&&... args) {
			std::string formatted = fmt::format(msg, std::forward<Args>(args)...);
			std::string finalMsg = fmt::format("Assertion failed in {}\n{}: {}", file, expression, formatted);
			RawLog(verbosity, Category::General, finalMsg);
			return finalMsg;
		}

		static std::string AssertLog(const char* expression, const char* file, Verbosity verbosity, const char* assertMsg) {
			std::string finalMsg = fmt::format("Assertion failed in {}\n{}: {}", file, expression, assertMsg);
			RawLog(verbosity, Category::General, finalMsg);
			return finalMsg;
		}

		static const std::vector<std::string>& GetErrorList() { return s_ErrorMessageStack; }


	private:
		static void RawLog(Verbosity verbosity, Category category, const std::string& msg);

	private:
		inline static bool s_Initialized = false;
		inline static std::mutex s_Mutex;
		inline static bool s_ColorEnabled = false;
		
		inline static std::vector<std::string> s_ErrorMessageStack;

		inline static std::vector<std::function<void(Verbosity, Category, const std::string&)>> s_LogCallbacks;

		inline static std::ofstream s_LogFile;
	};
		

#if defined(DEBUG) || defined(DEVELOPMENT)

#define SE_TRACE(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Trace, __VA_ARGS__)
#define SE_INFO(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Info, __VA_ARGS__)
#define SE_WARN(...)			Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Warning, __VA_ARGS__)
#define SE_ERR(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Error, __VA_ARGS__)

#else

#define SE_TRACE(...)			{}
#define SE_INFO(...)			{}
#define SE_WARN(...)			{}
#define SE_ERR(...)				Sphynx::Logging::Log(Sphynx::Logging::Verbosity::Error, __VA_ARGS__)

#endif
}