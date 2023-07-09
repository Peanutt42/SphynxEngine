#include "pch.hpp"
#include "Platform.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <debugapi.h>
#include <commdlg.h>
#include <shlobj.h>

namespace Sphynx {
	bool Platform::IsDebuggerAttached() {
		return IsDebuggerPresent();
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


	std::filesystem::path Platform::FileDialogs::OpenFile(const wchar_t* filter) {
		OPENFILENAMEW ofn;
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = nullptr;
		wchar_t szFile[260] = { 0 };
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST |
			OFN_FILEMUSTEXIST |
			OFN_NOCHANGEDIR;

		if (GetOpenFileNameW(&ofn))
			return ofn.lpstrFile;

		return {};
	}

	std::filesystem::path Platform::FileDialogs::SaveFile(const wchar_t* filter) {
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
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = std::wcschr(filter, '\0') + 1;

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
}

#endif