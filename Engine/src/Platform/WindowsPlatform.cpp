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
#pragma comment(lib, "Dbghelp.lib")

namespace Sphynx {
	bool Platform::IsDebuggerAttached() {
		return IsDebuggerPresent();
	}


	void Platform::SetWorkingDirToExe() {
		std::wstring filepathStr;
		filepathStr.resize(MAX_PATH);
		GetModuleFileNameW(nullptr, filepathStr.data(), filepathStr.size());
		std::filesystem::path filepath = filepathStr;
		std::filesystem::current_path(filepath.parent_path());
	}


	void Platform::MessagePrompts::Info(const std::string_view title, const std::string_view msg) {
		MessageBoxA(NULL, msg.data(), title.data(), MB_USERICON | MB_OK);
	}

	void Platform::MessagePrompts::Error(const std::string_view title, const std::string_view msg) {
		MessageBoxA(NULL, msg.data(), title.data(), MB_ICONERROR | MB_OK);
	}

	bool Platform::MessagePrompts::YesNo(const std::string_view title, const std::string_view msg) {
		return MessageBoxA(NULL, msg.data(), title.data(), MB_ICONQUESTION | MB_YESNO) == IDYES;
	}


	std::filesystem::path Platform::FileDialogs::OpenFile(const std::string& filterName, const std::string& filter) {
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

	std::filesystem::path Platform::FileDialogs::SaveFile(const std::string& filterName, const std::string& filter) {
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

	std::filesystem::path Platform::FileDialogs::OpenFolder() {
		if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
			return {};

		IFileOpenDialog* pFileOpenDialog = nullptr;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpenDialog));
		if (SUCCEEDED(hr)) {
			DWORD dwOptions;
			pFileOpenDialog->GetOptions(&dwOptions);
			pFileOpenDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);

			if (SUCCEEDED(pFileOpenDialog->Show(nullptr))) {
				IShellItem* pItem = NULL;
				if (SUCCEEDED(pFileOpenDialog->GetResult(&pItem))) {
					PWSTR pszFolderPath = nullptr;
					if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath))) {
						std::filesystem::path out = pszFolderPath;

						CoTaskMemFree(pszFolderPath);
						return out;
					}
					pItem->Release();
				}
			}

			pFileOpenDialog->Release();
		}

		CoUninitialize();

		return {};
	}



	void Platform::Process::Run(const std::filesystem::path& filepath, const std::wstring& args) {
		if (!std::filesystem::exists(filepath)) {
			SE_FATAL(Logging::General, "Can't find process to start in {}", filepath.string());
			return;
		}

		STARTUPINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi;

		ZeroMemory(&pi, sizeof(pi));

		std::wstring _args = filepath.native() + L" " + args;
		if (CreateProcessW(nullptr, _args.data(), nullptr, nullptr, false, DETACHED_PROCESS, nullptr, nullptr, &si, &pi)) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return;
		}
		else
			SE_FATAL(Logging::General, "Failed to start process {}", filepath.string());
	}

	unsigned long Platform::Process::GetCurrentProcessId() {
		return ::GetCurrentProcessId();
	}

	std::string Platform::Process::GetCurrentName() {
		char buffer[MAX_PATH];
		GetModuleBaseNameA(GetCurrentProcess(), GetModuleHandleA(nullptr), buffer, MAX_PATH);
		return buffer;
	}


	unsigned int Platform::Thread::GetCurrentId() {
		return ::GetCurrentThreadId();
	}


	StackTrace Platform::GenerateStackTrace() {
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
					trace.emplace_back("Internal function: abort()", false, "internal", 0);
				else if (symbolName == "raise")
					trace.emplace_back("Internal function: raise()", false, "internal", 0);
				else if (symbolName == "Sphynx::CrashHandler::OnProcessCrashed" ||
					symbolName == "Sphynx::CrashHandler::MakeStackTrace" ||
					symbolName == "Sphynx::Platform::GenerateStackTrace" ||
					symbolName == "`Sphynx::CrashHandler::Init'::`2'::<lambda_2>::operator()" ||
					symbolName == "`Sphynx::CrashHandler::Init'::`2'::<lambda_2>::<lambda_invoker_cdecl>" ||
					symbolName == "_seh_filter_exe" ||
					symbolName == "`__scrt_common_main_seh'::`1'::filt$0" ||
					symbolName == "__C_specific_handler" ||
					symbolName == "__chkstk" ||
					symbolName == "log2f" ||
					symbolName == "RtlFindCharInUnicodeString" ||
					symbolName == "KiUserExceptionDispatcher")
				{
					continue;
				}
				else if (symbolName == "CxxThrowException") {
					trace.clear(); // Everything until now has been internal c++ std::exception handeling
					trace.emplace_back("unhandeled std::exception", false, "internal", 0);
				}
				else {
					auto& entry = trace.emplace_back();
					entry.FunctionName = std::string(symbol->Name) + "()";

					if (SymGetLineFromAddr64(process, address, &displacement32, &line)) {
						entry.HasSource = true;
						entry.SourceFile = std::string(line.FileName);
						entry.SourceLine = (size_t)line.LineNumber;
					}
					else
						entry.HasSource = false;

					if (symbolName == "main")
						break;
				}
			}
			else {
				std::stringstream addressHex;
				addressHex << "0x" << std::hex << std::uppercase << symbol->Address;
				trace.emplace_back("Unknown symbol - " + addressHex.str(), false);
			}
		}
		SymCleanup(process);

		return trace;
	}


	struct Platform::DLLPlatformData {
		HMODULE Module = nullptr;
	};
	Platform::DynamicLinkLibary::DynamicLinkLibary(const std::filesystem::path& filepath) {
		m_PlatformData = new DLLPlatformData();

		SE_ASSERT(std::filesystem::exists(filepath), "{} doesn't exist!", filepath.string());

		m_PlatformData->Module = LoadLibraryW(filepath.native().c_str());
		SE_ASSERT(m_PlatformData->Module, "Failed to open {}", filepath.string());
	}

	Platform::DynamicLinkLibary::~DynamicLinkLibary() {
		delete m_PlatformData;
	}

	void* Platform::DynamicLinkLibary::_GetFuncAddress(const char* name) {
		return GetProcAddress(m_PlatformData->Module, name);
	}
}

#endif