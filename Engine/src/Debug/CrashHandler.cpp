#include "pch.hpp"
#include "CrashHandler.hpp"
#include "Core/Engine.hpp"
#include "Logging/Logging.hpp"
#include "Platform/Platform.hpp"
#include "Serialization/YAMLSerializer.hpp"

#include <csignal>

namespace Sphynx {
	void CrashHandler::Init() {
		if (Platform::IsDebuggerAttached())
			return;
		
		std::signal(SIGILL, OnProcessCrashed);  // illegal instruction
		std::signal(SIGSEGV, OnProcessCrashed); // segmentation fault
		std::signal(SIGABRT, OnProcessCrashed); // abort()

		Platform::Process::Run("Programs/CrashReporter/bin/CrashReporter.exe", std::to_wstring(Platform::Process::GetCurrentProcessId()));
	}

	void CrashHandler::OnProcessCrashed(int signal) {
		std::cout << "A crash occurred.\n";

		switch (signal) {
		default: std::cout << "unkown signal: " << signal << '\n'; break;
		case -1: break;
		case SIGILL: std::cout << "Illegal instruction!\n"; break;
		case SIGSEGV: std::cout << "Segmentation fault!\n"; break;
		case SIGABRT: std::cout << "abort() / throw was called!\n"; break;
		}

		StackTrace stackTrace = Platform::GenerateStackTrace();
		for (size_t i = 0; i < stackTrace.Entries.size(); i++) {
			auto& entry = stackTrace.Entries[i];
			std::cout << "[" + std::to_string(i + 1) + "] At " + entry.FunctionName << " in ";
			std::cout << (entry.HasSource ? entry.SourceFile + ":" + std::to_string(entry.SourceLine) : "???");
			std::cout << std::endl;
		}

		YAML::Emitter out;
		out << YAML::BeginMap;

		out << YAML::Key << "OS" << YAML::Value;
#ifdef WINDOWS
		out << "Windows";
#elif defined(LINUX)
		out << "Linux";
#elif defined(MACOS)
		out << "Macos";
#else
		out << "Unkown";
#endif

		out << YAML::Key << "Archetecture" << YAML::Value;
		if constexpr (sizeof(void*) == 8)
			out << "64 bit";
		else
			out << "32 bit";


		out << YAML::Key << "Crash" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Process" << YAML::Value << Platform::Process::GetCurrentName();
		out << YAML::Key << "Thread" << YAML::Value << std::to_string(Platform::Thread::GetCurrentId());
		out << YAML::Key << "LastErrorMessages" << YAML::Value << YAML::BeginSeq;
		for (const std::string& error : Logging::GetErrorList())
			out << YAML::Value << error;
		out << YAML::EndSeq;
		out << YAML::Key << "Signal" << YAML::Value;
		switch (signal) {
		default: out << ("unkown signal: " + std::to_string(signal)); break;
		case -1:	out << "Explicit decision!"; break;
		case SIGILL: out << "Illegal instruction!"; break;
		case SIGSEGV: out << "Segmentation fault!"; break;
		case SIGABRT: out << "abort() was called!"; break;
		}
		out << YAML::EndMap;


		out << YAML::Key << "Stacktrace" << YAML::Value;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < stackTrace.Entries.size(); i++) {
			auto& entry = stackTrace.Entries[i];
			out << YAML::BeginMap;
			out << YAML::Key << "FunctionName" << YAML::Value << entry.FunctionName;
			out << YAML::Key << "HasSource" << YAML::Value << entry.HasSource;
			if (entry.HasSource) {
				out << YAML::Key << "SourceFile" << YAML::Value << entry.SourceFile;
				out << YAML::Key << "SourceLine" << YAML::Value << entry.SourceLine;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		out << YAML::EndMap;
		Serialization::SaveYamlToFile("Programs/CrashReporter/CrashReport.txt", out);

		std::cin.get();

		std::exit(666); // Anything other than 0 exit code is caught by CrashReporter.exe
	}
}