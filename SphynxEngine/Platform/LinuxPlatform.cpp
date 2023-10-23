#include "pch.hpp"
#include "Platform.hpp"
#include "Logging/Logging.hpp"

#ifdef LINUX
#include <signal.h>
#include <dlfcn.h>

namespace Sphynx::Platform {
    bool IsDebuggerAttached() {
        return false;
    }

    bool ConsoleSupportsColor() {
        const char* term = std::getenv("TERM");
        if (term)
            return std::string_view(term).find("xterm") != std::string::npos;
        
        return false;
    }

    void SetWorkingDirToExe() {
        std::string exePath;
        exePath.resize(1024);
        ssize_t pathLen = readlink("/proc/self/exe", exePath.data(), exePath.size());
        if (pathLen != -1 && std::filesystem::exists(exePath))
            std::filesystem::current_path(std::filesystem::path(exePath).parent_path());
    }

    SE_API ExceptionCallback s_ExceptionCallback;

    std::string SingalToString(int sig) {
        switch (sig) {
        default:        return "Unkown Signal: " + std::to_string(sig);
        case SIGINT:    return "Interactive attention signal";
        case SIGILL:    return "Illegal instruction";
        case SIGABRT:   return "Abnormal termination";
        case SIGSEGV:   return "Invalid access to memory (nullptr, etc)";
        case SIGTERM:   return "Termination (abnormal)";
        }
    }

    void SignalHandler(int sig) {
        const std::string reason = SingalToString(sig);
        if (s_ExceptionCallback)
            s_ExceptionCallback(reason, nullptr);
        else
            SE_FATAL("Unset ExceptionCallback: Legacy Callback: Reason for crash: {}", reason);
    }

    void SetExceptionCallback(const ExceptionCallback& callback) {
        signal(SIGINT, SignalHandler);
        signal(SIGILL, SignalHandler);
        signal(SIGABRT, SignalHandler);
        signal(SIGSEGV, SignalHandler);
        signal(SIGTERM, SignalHandler);
        s_ExceptionCallback = callback;
    }


    // TODO: TEMP
	void MessagePrompts::Info(std::string_view title, std::string_view msg) {
        std::string cmd = fmt::format("zenity --info --title=\"{}\" --text=\"{}\"", title, msg);
        int returnCode = system(cmd.c_str());
        if (returnCode != 0)
            SE_WARN("Failed to display info message box: title: {}, msg: {}", title, msg);
	}
	void MessagePrompts::Error(std::string_view title, std::string_view msg) {
        std::string cmd = fmt::format("zenity --error --title=\"{}\" --text=\"{}\"", title, msg);
        int returnCode = system(cmd.c_str());
        if (returnCode != 0)
            SE_WARN("Failed to display error message box: title: {}, msg: {}", title, msg);
	}
	bool MessagePrompts::YesNo(std::string_view title, std::string_view msg) {
		std::string cmd = fmt::format("zenity --question --title=\"{}\" --text=\"{}\"", title, msg);
        int returnCode = system(cmd.c_str());
        if (returnCode == 0) return true;
        if (returnCode == 256) return false;
        
        SE_WARN("Failed to display error message box: title: {}, msg: {}", title, msg);
        return false;
    }

    // TODO: TEMP
    std::filesystem::path FileDialogs::OpenFile(const std::string& filterName, const std::string& filter) {
        FILE* file = popen("zenity --file-selection --title=\"File Selection\"", "r");
        if (!file)
            return {};

        char buffer[4096];
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {
            buffer[strcspn(buffer, "\n")] = '\0';
            std::filesystem::path path = buffer;
            pclose(file);
            return path;
        }
        pclose(file);
        return {};
    }
    std::filesystem::path FileDialogs::SaveFile(const std::string& filterName, const std::string& filter) {
        FILE* file = popen("zenity --file-selection --title=\"File Selection\"", "r");
        if (!file)
            return {};

        char buffer[4096];
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {
            buffer[strcspn(buffer, "\n")] = '\0';
            std::filesystem::path path = buffer;
            pclose(file);
            return path;
        }
        pclose(file);
        return {};
    }


    bool Process::Run(const std::filesystem::path& filepath, const std::vector<std::string>& args) {
        if (!std::filesystem::exists(filepath)) {
            SE_WARN("Failed to run executable {}", filepath.string());
            return false;
        }
        
        pid_t child_pid = fork();
        if (child_pid == 0) {
            // inside of the new process
            std::string filepathStr = filepath;
            std::vector<char*> argv(args.size() + 2);
            argv.front() = (char*)filepathStr.c_str();

            // offset by 1 since the first argument is the command (filepath to the executable)
            for (size_t i = 0; i < args.size(); i++)
                argv[i + 1] = (char*)args[i].data();

            argv.back() = nullptr;

            execvp(filepathStr.c_str(), argv.data());
        }
        else if (child_pid == -1)
            return false;
        
        return true;
    }

    unsigned long Process::GetCurrentProcessId() {
        return getpid();
    }

    std::string Process::GetCurrentName() {
        char exePath[1024] = {0};
        ssize_t pathLen = readlink("/proc/self/exe", exePath, sizeof(exePath));
        if (pathLen != -1 && std::filesystem::exists(exePath))
            return std::filesystem::path(exePath).filename().string();
        else
            return "ProcessNameNotFound.exe";
    }


    bool DynamicLinkLibrary::_Open() {
		if (!std::filesystem::exists(m_Filepath))
			return false;

		m_PlatformHandle = dlopen(m_Filepath.string().c_str(), RTLD_LAZY);
		if (!m_PlatformHandle)
			return false;

		return true;
	}

	void DynamicLinkLibrary::_Close() {
		if (dlclose(m_PlatformHandle) == 0)
			m_PlatformHandle = nullptr;
		else
			SE_WARN("Failed to close dll: {}", m_Filepath.string());
	}

	void* DynamicLinkLibrary::_GetFuncAddress(const char* name) {
		return dlsym(m_PlatformHandle, name);
	}

	const char* DynamicLinkLibrary::DLLExtension() {
		return ".so";
	}
	bool DynamicLinkLibrary::IsDLL(const std::filesystem::path& filepath) {
		return filepath.extension() == ".so";
	}
}

#endif