#include "pch.hpp"
#include "CrashHandler.hpp"
#include "Core/Engine.hpp"
#include "Logging/Logging.hpp"
#include "Serialization/YAMLSerializer.hpp"

#include <backward.hpp>

#include <csignal>

namespace Sphynx {
	void AbortHandler(int);
	void InvalidParameterHandler(const wchar_t* Expression, const wchar_t* Function, const wchar_t* File, uint32 Line, uintptr Reserved);
	void OnProcessCrashed(const std::string& reason, bool msgBox, void* context = nullptr);

	void CrashHandler::Init() {
		if (s_Initialized)
			return;

		if (Platform::IsDebuggerAttached())
			return;
		
		Platform::SetExceptionCallback([](const std::string& reason, void* context) {
			OnCrash(reason, true);
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
		CrashHandler::OnCrash(std::format("{} in {} in {}:{}", expressionStr, functionStr, fileStr, line), true);
	}


	void AbortHandler(int) {
		CrashHandler::OnCrash("abort() was called", true);
	}

	void CrashHandler::OnCrash(const std::string& reason, bool msgBox) {
		OnProcessCrashed(reason, msgBox);
	}

	void OnProcessCrashed(const std::string& reason, bool msgBox, void* context) {
		if (msgBox)
			Platform::MessagePrompts::Error("Engine crashed", reason);
		
		std::cout << "A crash occurred.\n";

		std::cout << reason << '\n';

		backward::StackTrace stacktrace;
		stacktrace.load_here(100, context);
	
		backward::TraceResolver stacktrace_resolver;
		stacktrace_resolver.load_stacktrace(stacktrace);
		
		for (size_t i = 0; i < stacktrace.size(); i++) {
			backward::ResolvedTrace trace = stacktrace_resolver.resolve(stacktrace[i]);
			std::cout << "[" + std::to_string(i + 1) + "] At ";
			if (!trace.object_filename.empty())
				std::cout << trace.object_filename << "::";
			std::cout << trace.source.function;
			if (!trace.source.filename.empty())
				std::cout << "\n    " << " in " << trace.source.filename + ":" + std::to_string(trace.source.line);
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
		out << YAML::Key << "MainThread" << YAML::Value << (std::this_thread::get_id() == Platform::s_MainThreadId);
		out << YAML::Key << "LastErrorMessages" << YAML::Value << YAML::BeginSeq;
		for (const std::string& error : Logging::GetErrorList())
			out << YAML::Value << error;
		out << YAML::EndSeq;
		out << YAML::Key << "Reason" << YAML::Value << reason;
		out << YAML::EndMap;


		out << YAML::Key << "Stacktrace" << YAML::Value;
		out << YAML::BeginSeq;
		for (size_t i = 0; i < stacktrace.size(); i++) {
			backward::ResolvedTrace trace = stacktrace_resolver.resolve(stacktrace[i]);
			out << YAML::BeginMap;
			out << YAML::Key << "FunctionName" << YAML::Value << trace.source.function;
			
			if (!trace.object_filename.empty())
				out << YAML::Key << "ModuleName" << YAML::Value << trace.object_filename;

			if (!trace.source.filename.empty()) {
				out << YAML::Key << "SourceFile" << YAML::Value << trace.source.filename;
				out << YAML::Key << "SourceLine" << YAML::Value << trace.source.line;
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