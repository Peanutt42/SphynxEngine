#pragma once

// This is not a forced way to write your main function but rather
// a way to have a safe GuardedMain function
// If you feel like it, just write your own main function

#include "pch.hpp"
#include "Engine.hpp"

// user defined guarded main function that is called after
// crash/exception handeling is installed
int GuardedMain(const int argc, const char** argv);

int main(const int argc, const char** argv) {
	Sphynx::CrashHandler::Init(); // Gets called by Engine::Init as well
	
	int errorCode = 0;
	try {
		errorCode = GuardedMain(argc, argv);
	}
	catch (const std::exception& e) {
		Sphynx::CrashHandler::OnCrash("std::exception was thrown: " + std::string(e.what()), true);
	}
	catch (...) {
		Sphynx::CrashHandler::OnCrash("unknown exception was thrown!", true);
	}
	return errorCode;
}