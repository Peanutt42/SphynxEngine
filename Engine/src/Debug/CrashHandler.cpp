#include "pch.hpp"
#include "CrashHandler.hpp"

#include "Core/Engine.hpp"
#include "Logging/Logging.hpp"
#include "Platform/Platform.hpp"
#include "Serialization/YAMLSerializer.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

namespace Sphynx {
	void CrashHandler::Init() {
		signal(SIGILL, OnProcessCrashed);  // illegal instruction
		signal(SIGSEGV, OnProcessCrashed); // segmentation fault
		signal(SIGABRT, OnProcessCrashed); // abort()

		if (!Platform::IsDebuggerAttached())
			Platform::Process::Run("Programs/CrashReporter/bin/CrashReporter.exe", std::to_wstring(Platform::Process::GetCurrentProcessId()));
	}

	StackTrace CrashHandler::MakeStackTrace(bool cutSetup) {
		StackTrace trace;

		constexpr int maxStackTraceDepth = 100;
		std::array<void*, maxStackTraceDepth> internalStacktraces;
		USHORT stackTraceSize = CaptureStackBackTrace(0, maxStackTraceDepth, internalStacktraces.data(), nullptr);

		HANDLE process = GetCurrentProcess();

		// Initialize symbol handling
		SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);
		SymInitialize(process, nullptr, TRUE);

		// Allocate space for symbol information
		constexpr DWORD maxSymbolNameLength = 256;
		char symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + maxSymbolNameLength];
		PIMAGEHLP_SYMBOL64 symbol = reinterpret_cast<PIMAGEHLP_SYMBOL64>(symbolBuffer);
		symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol->MaxNameLength = maxSymbolNameLength;


		IMAGEHLP_LINE64 line;
		ZeroMemory(&line, sizeof(line));
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

		DWORD displacement32 = 0;

		for (USHORT i = 0; i < stackTraceSize; ++i) {
			DWORD64 address = reinterpret_cast<DWORD64>(internalStacktraces[i]);
			if (SymGetSymFromAddr64(process, address, nullptr, symbol)) {
				std::string_view symbolName = symbol->Name;
				if (symbolName == "abort")
					trace.Entries.emplace_back("Internal function: abort()", false, "internal", 0);
				else if (symbolName == "raise")
					trace.Entries.emplace_back("Internal function: raise()", false, "internal", 0);
				else if (symbolName == "Sphynx::CrashHandler::OnProcessCrashed" ||
						 symbolName == "Sphynx::CrashHandler::MakeStackTrace" ||
						 symbolName == "`Sphynx::CrashHandler::Init'::`2'::<lambda_2>::operator()" ||
						 symbolName == "`Sphynx::CrashHandler::Init'::`2'::<lambda_2>::<lambda_invoker_cdecl>" ||
						 symbolName == "_seh_filter_exe" ||
						 symbolName == "`__scrt_common_main_seh'::`1'::filt$0" ||
						 symbolName == "__C_specific_handler" ||
						 symbolName == "__chkstk" ||
						 symbolName == "log2f" ||
						 symbolName == "RtlFindCharInUnicodeString" ||
						 symbolName == "KiUserExceptionDispatcher")
					continue;
				else {
					auto& entry = trace.Entries.emplace_back();
					entry.FunctionName = std::string(symbol->Name) + "()";

					if (SymGetLineFromAddr64(process, address, &displacement32, &line)) {
						entry.HasSource = true;
						entry.SourceFile = std::string(line.FileName);
						entry.SourceLine = (size_t)line.LineNumber;
					}
					else
						entry.HasSource = false;

					if (cutSetup && symbolName == "main")
						break;
				}
			}
			else {
				std::stringstream addressHex;
				addressHex << "0x" << std::hex << std::uppercase << symbol->Address;
				trace.Entries.emplace_back("Unknown symbol - " + addressHex.str(), false);
			}
		}
		SymCleanup(process);

		return trace;
	}

	void CrashHandler::OnProcessCrashed(int signal) {
		std::cout << "A crash occurred.\n";

		switch (signal) {
		default: std::cout << "unkown signal: " << signal << '\n'; break;
		case -1: break;
		case SIGILL: std::cout << "Illegal instruction!\n"; break;
		case SIGSEGV: std::cout << "Segmentation fault!\n"; break;
		case SIGABRT: std::cout << "abort() was called!\n"; break;
		}

		StackTrace stackTrace = MakeStackTrace(true);
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