#include "pch.hpp"
#include "Platform.hpp"
#include "Logging/Logging.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <debugapi.h>
#include <commdlg.h>
#include <shlobj.h>
#include <Psapi.h>
#include <DbgHelp.h>

namespace Sphynx::Platform {
	bool IsDebuggerAttached() {
		return IsDebuggerPresent();
	}


	bool ConsoleSupportsColor() {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE)
			return false;
		
		DWORD consoleMode;
		if (!GetConsoleMode(hOut, &consoleMode))
			return false;

		return consoleMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	}


	void SetWorkingDirToExe() {
		std::wstring filepathStr;
		filepathStr.resize(MAX_PATH);
		GetModuleFileNameW(nullptr, filepathStr.data(), (DWORD)filepathStr.size());
		std::filesystem::path filepath = filepathStr;
        if (std::filesystem::exists(filepath))
            std::filesystem::current_path(filepath.parent_path());
	}


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

			if (record->ExceptionInformation[1] == (ULONG)0)
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
		case EXCEPTION_FLT_DENORMAL_OPERAND: return "float denormal operand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "float divide by zero";
		case EXCEPTION_FLT_INVALID_OPERATION: return "float invalid operation";
		case EXCEPTION_FLT_OVERFLOW: return "float overflow";
		case EXCEPTION_FLT_UNDERFLOW: return "float underflow";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "int divide by zero";
		case EXCEPTION_INT_OVERFLOW: return "int overflow";
		}
	}

	static ExceptionCallback s_ExceptionCallback;

	LONG WINAPI ExceptionFilter(PEXCEPTION_POINTERS exceptionInfo) {
		std::string reason = ExceptionToStr(exceptionInfo->ExceptionRecord);
		if (s_ExceptionCallback)
			s_ExceptionCallback(reason, exceptionInfo->ContextRecord);
		else
			SE_FATAL("Unset ExceptionCallback: Legacy Callback: Reason for crash: {}", reason);

		// doesn't matter since we have already quit by now
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void SetExceptionCallback(const ExceptionCallback& callback) {
		SetUnhandledExceptionFilter(ExceptionFilter);
		s_ExceptionCallback = callback;
	}


	void MessagePrompts::Info(std::string_view title, std::string_view msg) {
		MessageBoxA(NULL, msg.data(), title.data(), MB_USERICON | MB_OK);
	}

	void MessagePrompts::Error(std::string_view title, std::string_view msg) {
		MessageBoxA(NULL, msg.data(), title.data(), MB_ICONERROR | MB_OK);
	}

	bool MessagePrompts::YesNo(std::string_view title, std::string_view msg) {
		return MessageBoxA(NULL, msg.data(), title.data(), MB_ICONQUESTION | MB_YESNO) == IDYES;
	}


	std::filesystem::path FileDialogs::OpenFile(const std::string& filterName, const std::string& filter) {
		// filter: "name (filter)\0filter\0"
		std::wstring wfilter = std::wstring(filter.begin(), filter.end());
		std::wstring finalFilter = std::wstring(filterName.begin(), filterName.end()) + L" (" + wfilter + L")\0" + wfilter + L'\0';

		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		wchar_t szFile[260] = { 0 };
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = finalFilter.c_str();
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST |
			OFN_FILEMUSTEXIST |
			OFN_NOCHANGEDIR;

		if (GetOpenFileNameW(&ofn))
			return ofn.lpstrFile;

		return {};
	}

	std::filesystem::path FileDialogs::SaveFile(const std::string& filterName, const std::string& filter) {
		// filter: "name (filter)\0filter\0"
		std::wstring wfilter = std::wstring(filter.begin(), filter.end());
		std::wstring finalFilter = std::wstring(filterName.begin(), filterName.end()) + L" (" + wfilter + L")\0" + wfilter + L'\0';

		OPENFILENAMEW ofn;
		wchar_t szFile[260] = { 0 };
		wchar_t currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryW(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = finalFilter.c_str();
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = std::wcschr(finalFilter.c_str(), '\0') + 1;

		if (GetSaveFileNameW(&ofn))
			return ofn.lpstrFile;

		return {};
	}


	bool Process::Run(const std::filesystem::path& filepath, const std::wstring& args) {
		if (!std::filesystem::exists(filepath)) {
			SE_ERR(Logging::General, "Can't find process to start in {}", filepath.string());
			return false;
		}

		STARTUPINFOW si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi;

		ZeroMemory(&pi, sizeof(pi));

		std::wstring _args = filepath.native() + L" " + args;
		if (CreateProcessW(nullptr, _args.data(), nullptr, nullptr, false, DETACHED_PROCESS, nullptr, nullptr, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return true;
		}
		else {
			SE_ERR(Logging::General, "Failed to start process {}", filepath.string());
			return false;
		}
	}

	unsigned long Process::GetCurrentProcessId() {
		return ::GetCurrentProcessId();
	}

	std::string Process::GetCurrentName() {
		char buffer[MAX_PATH];
		GetModuleBaseNameA(GetCurrentProcess(), GetModuleHandleA(nullptr), buffer, MAX_PATH);
		return buffer;
	}


	unsigned int Thread::GetCurrentId() {
		return ::GetCurrentThreadId();
	}


	bool DynamicLinkLibrary::_Open() {
		if (!std::filesystem::exists(m_Filepath))
			return false;

		m_PlatformHandle = LoadLibraryW(m_Filepath.native().c_str());
		if (!m_PlatformHandle)
			return false;

		return true;
	}

	void DynamicLinkLibrary::_Close() {
		if (FreeLibrary((HMODULE)m_PlatformHandle))
			m_PlatformHandle = nullptr;
		else
			SE_WARN("Failed to close dll: {}", m_Filepath.string());
	}

	void* DynamicLinkLibrary::_GetFuncAddress(const char* name) {
		return GetProcAddress((HMODULE)m_PlatformHandle, name);
	}

	const char* DynamicLinkLibrary::DLLExtension() {
		return ".dll";
	}
	bool DynamicLinkLibrary::IsDLL(const std::filesystem::path& filepath) {
		return filepath.extension() == ".dll";
	}
}

#endif