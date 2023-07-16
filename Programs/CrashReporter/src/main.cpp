#pragma warning(disable: 4005) // redefinition of APIENTRY

#include "CrashReporterGUI.hpp"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>
#include <iostream>

using namespace Sphynx;

void crash(std::string_view msg) {
	std::cout << msg << '\n';
	std::cin.get();
	std::exit(1);
}

int main(const int argc, const char** argv) {
	if (argc < 1) {
		crash("There should always be atleast one cmd arg");
	}

	std::filesystem::current_path(std::filesystem::path(argv[0]).parent_path().parent_path());

	if (argc != 2) {
		crash("Usage: [process_id]");
	}
	unsigned int processId = 0;

	try {
		processId = std::stoi(argv[1]);
	}
	catch (const std::exception& e) {
		crash(std::string("Failed to parse process_id argument: ") + e.what());
	}

	std::cout << "Attaching to process: " << processId << '\n';
	
	// Connect to main app
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (!process) {
		crash(std::string("Failed to open processId: ") + std::to_string(processId) + '\n' + "Possible fixes:\n"
					 "  - invalid processId\n"
					 "  - access refused\n");
	}

	// Wait for process to exit
	unsigned long result;
	while (true) {
		result = WaitForSingleObject(process, ULONG_MAX);
		if (result != WAIT_TIMEOUT && result != WAIT_FAILED)
			break;
		if (result == WAIT_FAILED)
			std::cout << "Failed to wait for process termination of procid: " << processId << '\n';
		std::cout << "Waiting..." << std::endl;
	}

	// Get exit code
	unsigned long exitCode;
	GetExitCodeProcess(process, &exitCode);
	std::cout << "Process exited with code " << exitCode << '\n';
	
	if (exitCode != 0) {
		std::cout << "Crash detected!\n";
		
		CrashReporterGUIRun();
	}
	else
		std::cout << "Main app finished correctly, closing CrashReporter...\n";

	return 0;
}
#endif