#include "pch.hpp"
#include "Logging.hpp"
#include "Platform/Platform.hpp"
#include "Profiling/Profiling.hpp"

#include <ctime>

namespace Sphynx {
	constexpr const char* GetColorForVerbosity(Logging::Verbosity verbosity) {
		switch (verbosity) {
		default: return "";
		case Logging::Verbosity::Trace:		return "[97m";
		case Logging::Verbosity::Info:		return "[92m";
		case Logging::Verbosity::Warning:	return "[33m";
		case Logging::Verbosity::Error:		return "[91m";
		case Logging::Verbosity::Critical:	return "[31m";
		}
	}

	const char* Logging::CategoryToString(Category category) {
		switch (category) {
		default:
		case Category::General:			return "[General]        ";
		case Category::Game:			return "[Game]           ";
		case Category::Editor:			return "[Editor]         ";
		case Category::Runtime:			return "[Runtime]        ";
		case Category::Audio:			return "[Audio]          ";
		case Category::AssetManagment:	return "[Assets]         ";
		case Category::Serialization:	return "[Serialization]  ";
		case Category::Memory:			return "[Memory]         ";
		case Category::Networking:		return "[Networking]     ";
		case Category::Scripting:		return "[Scripting]      ";
		case Category::ECS:				return "[ECS]            ";
		case Category::Physics:			return "[Physics]        ";
		case Category::Rendering:		return "[Rendering]      ";
		case Category::UI:				return "[UI]             ";
		case Category::Building:		return "[Building]       ";
		}
	}

	const char* Logging::VerbosityToString(Verbosity verbosity) {
		switch (verbosity) {
		default:					return "[INVALID] ";
		case Verbosity::Trace:		return "[Trace]   ";
		case Verbosity::Info:		return "[Info]    ";
		case Verbosity::Warning:	return "[Warning] ";
		case Verbosity::Error:		return "[Error]   ";
		case Verbosity::Critical:	return "[Critical]";
		}
	}


	void Logging::Init() {
		SE_PROFILE_FUNCTION();
		
		if (s_Initialized)
			return;

		std::scoped_lock lock(s_Mutex);

		s_ColorEnabled = Platform::ConsoleSupportsColor();

		s_LogFile.open("Engine.log");
		if (!s_LogFile.is_open())
			CrashHandler::OnCrash("Failed to create Logfile!");

		s_Initialized = true;
	}

	void Logging::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!s_Initialized)
			return;

		std::scoped_lock lock(s_Mutex);

		s_LogFile.close();

		s_Initialized = false;
	}

	void Logging::RawLog(Verbosity verbosity, Category category, const std::string& msg) {
		if (!s_Initialized)
			Init();
		
		const char* categoryStr = CategoryToString(category);
		const char* verbosityStr = VerbosityToString(verbosity);

		std::scoped_lock lock(s_Mutex);
		if (verbosity != Verbosity::Trace) {
			if (s_ColorEnabled)
				std::cout << '\033' << GetColorForVerbosity(verbosity);
			else
				std::cout << verbosityStr;

			std::cout << categoryStr << msg;

			if (s_ColorEnabled)
				std::cout << "\033[0m\n";
			else
				std::cout << '\n';

			for (const auto& callback : s_LogCallbacks)
				callback(verbosity, category, msg);
		}

		if (verbosity == Verbosity::Critical)
			Platform::MessagePrompts::Error("Sphynx Engine Error", msg);

		std::time_t time = std::time(nullptr);
    	std::tm* timeinfo = std::localtime(&time);

		std::string loggedMsg = fmt::format("{}:{}:{}: {} {} {}", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, verbosityStr, categoryStr, msg);
		s_LogFile << loggedMsg << '\n';

		if (verbosity >= Verbosity::Error)
			s_ErrorMessageStack.push_back(loggedMsg);
	}
}