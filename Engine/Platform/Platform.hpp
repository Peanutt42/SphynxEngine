#pragma once

#include "std.hpp"
#include "Logging/Logging.hpp"
#include "Debug/StackTrace.hpp"

namespace Sphynx::Platform {
	bool IsDebuggerAttached();

    bool ConsoleSupportsColor();

    void SetWorkingDirToExe();

    // context argument is only windows specific and not needed when not inside crash handeling
    StackTrace GenerateStackTrace(void* customContext = nullptr);

    // context is only windows specific
    using ExceptionCallback = std::function<void(const std::string& reason, void* context)>;
    void SetExceptionCallback(const ExceptionCallback& callback);

    namespace MessagePrompts {
        void Info(std::string_view title, std::string_view msg);
        void Error(std::string_view title, std::string_view msg);
        bool YesNo(std::string_view title, std::string_view msg);
    }

    namespace FileDialogs {
        std::filesystem::path OpenFile(const std::string& filterName, const std::string& filter);
        std::filesystem::path SaveFile(const std::string& filterName, const std::string& filter);

        std::filesystem::path OpenFolder();
    }


    namespace Process {
        bool Run(const std::filesystem::path& filepath, const std::wstring& args);

        unsigned long GetCurrentProcessId();

        std::string GetCurrentName();
    }


    namespace Thread {
        unsigned int GetCurrentId();
    };


    struct DLLPlatformData;
    class DynamicLinkLibary {
    public:
        DynamicLinkLibary(const std::filesystem::path& filepath);
        ~DynamicLinkLibary();

        template<typename Func>
        std::optional<Func> LoadFunction(const std::string_view name) {
            if (!m_PlatformData)
                return std::nullopt;
            Func function = (Func)_GetFuncAddress(name.data());
            if (!function)
                return std::nullopt;
            return function;
        }

        static bool IsDLL(const std::filesystem::path& filepath);
        // returns extension with the . on the left: example: ".dll"
        static const char* DLLExtension();

    private:
        void* _GetFuncAddress(const char* name);

        DynamicLinkLibary(const DynamicLinkLibary&) = delete;
        DynamicLinkLibary(DynamicLinkLibary&&) = delete;
        DynamicLinkLibary& operator=(const DynamicLinkLibary&) = delete;
        DynamicLinkLibary& operator=(DynamicLinkLibary&&) = delete;

    private:
        DLLPlatformData* m_PlatformData = nullptr;
        std::filesystem::path m_Filepath;
    };


    inline static std::string WideToNarrow(const std::wstring& wstr) {
        std::locale loc("");
        std::string result(wstr.size(), 0);
        for (size_t i = 0; i < result.size(); i++)
            result[i] = std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr[i], '?');
        return result;
    }

    inline static std::wstring NarrowToWide(const std::string& str) {
        std::locale loc("");
        std::wstring result(str.size(), 0);
        for (size_t i = 0; i < result.size(); i++)
            result[i] = std::use_facet<std::ctype<wchar_t>>(loc).widen(str[i]);
        return result;
    }
}