#include "pch.hpp"
#include "CrashHandler.hpp"
#include "StackTrace.hpp"
#include "Core/Engine.hpp"
#include "Logging/Logging.hpp"
#include "Serialization/YAMLSerializer.hpp"

#include <vulkan/vulkan.hpp> // for vk::Error

#include <csignal>

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>
#include <Psapi.h>
#endif

namespace Sphynx {
	void AbortHandler(int);
	void InvalidParameterHandler(const wchar_t* Expression, const wchar_t* Function, const wchar_t* File, uint32_t Line, uintptr_t Reserved);
	void OnProcessCrashed(const std::string& reason, PCONTEXT pcontext = nullptr, bool msgBox = false);


#ifdef WINDOWS
#define CPP_EXCEPTION_CODE (0xE06D7363)

	std::string ExceptionToStr(PEXCEPTION_RECORD record) {
		switch (record->ExceptionCode) {
		default: return "Unknown exceptionCode " + std::to_string(record->ExceptionCode);
		case EXCEPTION_ACCESS_VIOLATION: {
			std::string output = "Access violation ";

			if (record->NumberParameters >= 1) {
				if (record->ExceptionInformation[0] == 0)
					output += "reading address ";
				else if (record->ExceptionInformation[0] == 1)
					output += "writing to address ";
				else if (record->ExceptionInformation[0] == 8)
					output += "trying to run function ptr at address ";
			}
			else
				output += "writing/reading address ";
		
			if (record->ExceptionInformation[1] == (ULONG)nullptr)
				output += "nullptr";
			else {
				std::stringstream addressHex;
				addressHex << "0x" << std::hex << std::uppercase << record->ExceptionInformation[1];
				output += addressHex.str();
			}
			return output;
		}		
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array index out of bounds";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal instruction";
		case EXCEPTION_STACK_OVERFLOW: return "Stack overflow";
		case EXCEPTION_BREAKPOINT: return "Breakpoint";
		case EXCEPTION_IN_PAGE_ERROR: return "In page error (maybe accessing invalid memory address)";
		case EXCEPTION_PRIV_INSTRUCTION: return "tried to execute privilegded/invalid instruction";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "tried to access a datatype without proper alignment";
		case CPP_EXCEPTION_CODE: return "C++ exception";
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "float denormal operand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "float divide by zero";
		case EXCEPTION_FLT_INVALID_OPERATION: return "float invalid operation";
		case EXCEPTION_FLT_OVERFLOW: return "float overflow";
		case EXCEPTION_FLT_UNDERFLOW: return "float underflow";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "int divide by zero";
		case EXCEPTION_INT_OVERFLOW: return "int overflow";
		}
	}

	LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS exceptionInfo) {
		if (exceptionInfo->ExceptionRecord->ExceptionCode == CPP_EXCEPTION_CODE) {
			Platform::MessagePrompts::Error("c++ except", "");
			std::cout << "C++ exception found\n";
			return EXCEPTION_CONTINUE_SEARCH; // cpp exceptions are handled by TerminateHandler
		}

		std::string reason = ExceptionToStr(exceptionInfo->ExceptionRecord);
		OnProcessCrashed(reason, exceptionInfo->ContextRecord, true);

		return EXCEPTION_CONTINUE_SEARCH;
	}
#endif

	void CrashHandler::Init() {
		if (s_Initialized)
			return;

		if (Platform::IsDebuggerAttached())
			return;
		
#ifdef WINDOWS
		SetUnhandledExceptionFilter(ExceptionFilter);
#endif

		_set_invalid_parameter_handler(InvalidParameterHandler);

		if (std::signal(SIGABRT, AbortHandler) == SIG_ERR)
			SE_ERR("Failed to register AbortHandler");

		s_Initialized = true;
	}

	void CrashHandler::StartCrashReporter() {
		if (Platform::IsDebuggerAttached())
			return;

		Platform::Process::Run("Programs/CrashReporter/bin/CrashReporter.exe", std::to_wstring(Platform::Process::GetCurrentProcessId()));
	}

	void InvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, uint32_t line, [[maybe_unused]] uintptr_t Reserved) {
		std::wstring expressionWStr = expression;
		std::wstring functionWStr = function;
		std::wstring fileWStr = file;
		std::string expressionStr{ expressionWStr.begin(), expressionWStr.end() };
		std::string functionStr{ functionWStr.begin(), functionWStr.end() };
		std::string fileStr{ fileWStr.begin(), fileWStr.end() };
		OnProcessCrashed(
			std::format("{} in {} in {}:{}", expressionStr, functionStr, fileStr, line),
			nullptr, true
		);
	}


	void AbortHandler(int) {
		OnProcessCrashed("abort() was called", nullptr, true);
	}

	void CrashHandler::OnCrash(const std::string reason, bool msgBox) {
		OnProcessCrashed(reason, nullptr, msgBox);
	}

	void OnProcessCrashed(const std::string& reason, PCONTEXT pcontext, bool msgBox) {
		if (msgBox)
			Platform::MessagePrompts::Error("Engine crashed", reason);
		
		std::cout << "A crash occurred.\n";

		std::cout << reason << '\n';

#ifdef WINDOWS
		StackTrace stacktrace;
				
		CONTEXT context;
		PCONTEXT finalContext = pcontext;
		if (!finalContext) {
			memset(&context, 0, sizeof(context));
			context.ContextFlags = CONTEXT_FULL;

			RtlCaptureContext(&context);
			finalContext = &context;
		}

		STACKFRAME64 stackFrame;
		memset(&stackFrame, 0, sizeof(stackFrame));

		DWORD machineType = IMAGE_FILE_MACHINE_I386;
#ifdef _M_X64
		machineType = IMAGE_FILE_MACHINE_AMD64;
#endif

		stackFrame.AddrPC.Mode = AddrModeFlat;
		stackFrame.AddrPC.Offset = finalContext->Rip;
		stackFrame.AddrFrame.Mode = AddrModeFlat;
		stackFrame.AddrFrame.Offset = finalContext->Rbp;
		stackFrame.AddrStack.Mode = AddrModeFlat;
		stackFrame.AddrStack.Offset = finalContext->Rsp;

		HANDLE process = GetCurrentProcess();
		HANDLE thread = GetCurrentThread();

		std::string currentProcessName = Platform::Process::GetCurrentName();
		// remove extension
		std::filesystem::path currentProcesName_path = currentProcessName;
		currentProcesName_path.replace_extension("");
		currentProcessName = currentProcesName_path.string();
		std::cout << currentProcessName << std::endl;

		SymInitialize(process, NULL, TRUE);

		while (StackWalk64(machineType, process, thread, &stackFrame, finalContext, nullptr,
			SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
		{
			char symbolBuffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_PATH];
			memset(symbolBuffer, 0, sizeof(symbolBuffer));
			IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)symbolBuffer;
			symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
			symbol->MaxNameLength = MAX_PATH;

			DWORD64 displacement = 0;
			if (SymGetSymFromAddr64(process, stackFrame.AddrPC.Offset, &displacement, symbol)) {
				std::string symbolName = symbol->Name;

				if (symbolName == __FUNCTION__) {
					stacktrace.clear();
					continue;
				}

				if (symbolName == "CxxThrowException") {
					stacktrace.clear();
					stacktrace.emplace_back("exception was thrown", false);
					continue;
				}

				StackTraceEntry& entry = stacktrace.emplace_back();
				entry.FunctionName = symbolName + "()";

				// Get module name if available

				IMAGEHLP_MODULE64 moduleInfo;
				std::memset(&moduleInfo, 0, sizeof(moduleInfo));
				moduleInfo.SizeOfStruct = sizeof(moduleInfo);
				if (SymGetModuleInfo64(process, stackFrame.AddrPC.Offset, &moduleInfo)) {
					std::filesystem::path moduleFilepath = moduleInfo.LoadedImageName;

					entry.ModuleName = moduleFilepath.filename().string();
					entry.HasModule = true;
				}
				else
					entry.HasModule = false;
					

				// Display source file and line number if available
				IMAGEHLP_LINE64 lineInfo;
				memset(&lineInfo, 0, sizeof(lineInfo));
				lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

				DWORD lineDisplacement = 0;
				if (SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &lineDisplacement, &lineInfo)) {
					entry.HasSource = true;
					entry.SourceFile = lineInfo.FileName;
					entry.SourceLine = lineInfo.LineNumber;
				}
				else
					entry.HasSource = false;

				if (symbolName == "main")
					break;
			}
			else {
				std::stringstream addressHex;
				addressHex << "0x" << std::hex << std::uppercase << symbol->Address;
				stacktrace.emplace_back("Unknown symbol - " + addressHex.str(), false);
			}
		}

		SymCleanup(process);
#endif
		
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
		YAMLSerializer::SaveFile("Programs/CrashReporter/CrashReport.txt", out);

		std::cout << "Press any key to exit program, which will launch the crash reporter\n";

		std::cin.get();

		Sphynx::Engine::Shutdown();

		std::exit(1);
	}
}