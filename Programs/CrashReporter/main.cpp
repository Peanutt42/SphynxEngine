#pragma warning(disable: 4005) // redefinition of APIENTRY

#include "CrashReporterGUI.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(LINUX)
#include <sys/wait.h>
#endif

#include <thread>
#include <filesystem>
#include <iostream>

#ifdef WINDOWS
bool ProcessExists(DWORD pid) {
	HANDLE process = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, false, pid);
	DWORD exitCode = 0;
	if (!GetExitCodeProcess(process, &exitCode))
		return false;
	CloseHandle(process);
	return exitCode == STILL_ACTIVE;
}
#elif defined(LINUX)
bool ProcessExists(pid_t pid) {
	while (waitpid(-1, nullptr, WNOHANG) > 0) {}
	return kill(pid, 0) == 0;
}
#endif

int main(const int argc, const char** argv) {
	if (argc != 2) {
		std::cout << "Usage: [pid]\n";
		return 1;
	}
	unsigned long pid = 0;
	try {
		pid = std::stoul(argv[1]);
	}
	catch (...) {
		std::cout << "pid isn't a valid number!";
		return 1;
	}

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

	while (ProcessExists(pid)) {
		if (std::filesystem::exists("CrashReport.txt")) {
			Sphynx::CrashReporterGUIRun();
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}