#pragma warning(disable: 4005) // redefinition of APIENTRY

#include "CrashReporterGUI.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <thread>
#include <filesystem>
#include <iostream>

int main(const int argc, const char** argv) {
#if defined(WINDOWS)
	std::wstring exeFilepathStr;
    exeFilepathStr.resize(MAX_PATH);
	GetModuleFileNameW(nullptr, exeFilepathStr.data(), (DWORD)exeFilepathStr.size());
	std::filesystem::path filepath = exeFilepathStr;
	if (std::filesystem::exists(filepath))
        std::filesystem::current_path(filepath.parent_path());

#elif defined(LINUX)

	std::string exePath;
	exePath.resize(1024);
	ssize_t pathLen = readlink("/proc/self/exe", exePath.data(), exePath.size());
	if (pathLen != -1 && std::filesystem::exists(exePath))
		std::filesystem::current_path(std::filesystem::path(exePath).parent_path());
#endif

	while (true) {
		if (std::filesystem::exists("CrashReport.txt"))
			break;
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	Sphynx::CrashReporterGUIRun();

	return 0;
}