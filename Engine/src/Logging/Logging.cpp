#include "pch.hpp"
#include "Logging.hpp"
#include "Platform/Platform.hpp"
#include "Profiling/Profiling.hpp"

namespace Sphynx {
	constexpr const char* CategoryToString(Logging::Category category) {
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


	void Logging::Init() {
		SE_PROFILE_FUNCTION();

		if (s_Initialized)
			return;

		std::scoped_lock lock(s_Mutex);

		if (!std::filesystem::exists("Logs"))
			std::filesystem::create_directory("Logs");

		s_LogFile.open("Logs/Engine.log");
		assert(s_LogFile.is_open() && "Failed to create Logfile!");

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
		std::cout << '\033' << GetColorForVerbosity(verbosity) << CategoryToString(category) << msg << "\033[0m\n";

		if (verbosity >= Verbosity::Critical)
			Platform::MessagePrompts::Error("Sphynx Engine Error", msg);

		if (!s_Initialized)
			Init();

		if (verbosity < s_Verbosity)
			return;
		
		std::scoped_lock lock(s_Mutex);
		s_LogFile << msg << '\n';

		if (verbosity >= Verbosity::Error)
			s_ErrorMessageStack.push_back(msg);
	}
}