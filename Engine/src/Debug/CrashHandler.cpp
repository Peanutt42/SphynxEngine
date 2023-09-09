#include "pch.hpp"
#include "CrashHandler.hpp"
#include "StackTrace.hpp"
#include "Core/Engine.hpp"
#include "Logging/Logging.hpp"
#include "Serialization/YAMLSerializer.hpp"

#include <csignal>

namespace Sphynx {
	void AbortHandler(int);
	void InvalidParameterHandler(const wchar_t* Expression, const wchar_t* Function, const wchar_t* File, uint32 Line, uintptr Reserved);
	void OnProcessCrashed(const std::string& reason, void* context = nullptr, bool msgBox = false);


	void CrashHandler::Init() {
		if (s_Initialized)
			return;

		if (Platform::IsDebuggerAttached())
			return;
		
		Platform::SetExceptionCallback([](const std::string& reason, void* context) {
			OnProcessCrashed(reason, context, true);
		});

		_set_invalid_parameter_handler(InvalidParameterHandler);

		if (std::signal(SIGABRT, AbortHandler) == SIG_ERR)
			SE_ERR("Failed to register AbortHandler");

		s_Initialized = true;
	}

	void CrashHandler::StartCrashReporter() {
		if (Platform::IsDebuggerAttached())
			return;

		Platform::Process::Run("CrashReporter.exe", std::to_wstring(Platform::Process::GetCurrentProcessId()));
	}

	void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, uint32 line, [[maybe_unused]] uintptr Reserved) {
		std::string expressionStr = Platform::WideToNarrow(expression);
		std::string functionStr = Platform::WideToNarrow(function);
		std::string fileStr = Platform::WideToNarrow(file);
		OnProcessCrashed(std::format("{} in {} in {}:{}", expressionStr, functionStr, fileStr, line), nullptr, true);
	}


	void AbortHandler(int) {
		OnProcessCrashed("abort() was called", nullptr, true);
	}

	void CrashHandler::OnCrash(const std::string reason, bool msgBox) {
		OnProcessCrashed(reason, nullptr, msgBox);
	}

	void OnProcessCrashed(const std::string& reason, void* context, bool msgBox) {
		if (msgBox)
			Platform::MessagePrompts::Error("Engine crashed", reason);
		
		std::cout << "A crash occurred.\n";

		std::cout << reason << '\n';

		StackTrace stacktrace = Platform::GenerateStackTrace(context);
		
		for (size_t i = 0; i < stacktrace.size(); i++) {
			auto& entry = stacktrace[i];
			std::cout << "[" + std::to_string(i + 1) + "] At ";
			if (entry.HasModule)
				std::cout << entry.ModuleName << "::";
			std::cout << entry.FunctionName;
			if (entry.HasSource)
				std::cout << "\n    " << " in " << entry.SourceFile + ":" + std::to_string(entry.SourceLine);
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
		out << YAML::Key << "Reason" << YAML::Value << reason;
		out << YAML::EndMap;


		out << YAML::Key << "Stacktrace" << YAML::Value;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < stacktrace.size(); i++) {
			auto& entry = stacktrace[i];
			out << YAML::BeginMap;
			out << YAML::Key << "FunctionName" << YAML::Value << entry.FunctionName;

			if (entry.HasModule)
				out << YAML::Key << "ModuleName" << YAML::Value << entry.ModuleName;

			if (entry.HasSource) {
				out << YAML::Key << "SourceFile" << YAML::Value << entry.SourceFile;
				out << YAML::Key << "SourceLine" << YAML::Value << entry.SourceLine;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		out << YAML::EndMap;
		YAMLSerializer::SaveFile("CrashReport.txt", out);

        Sphynx::Logging::Shutdown(); // make sure log files are finished

		std::cout << "Press any key to exit program, which will launch the crash reporter\n";

		std::cin.get();

		std::exit(1);
	}
}