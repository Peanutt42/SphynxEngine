#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"
#include "Logging/Logging.hpp"
#include "Debug/StackTrace.hpp"

namespace Sphynx::Platform {
	SE_API bool IsDebuggerAttached();

    SE_API bool ConsoleSupportsColor();

    SE_API void SetWorkingDirToExe();

    SE_API float GetCPUUsage();

    // context argument is only windows specific and not needed when not inside crash handeling
    SE_API StackTrace GenerateStackTrace(void* customContext = nullptr);

    // context is only windows specific
    using ExceptionCallback = std::function<void(const std::string& reason, void* context)>;
    SE_API void SetExceptionCallback(const ExceptionCallback& callback);

    namespace MessagePrompts {
        SE_API void Info(std::string_view title, std::string_view msg);
        SE_API void Error(std::string_view title, std::string_view msg);
        SE_API bool YesNo(std::string_view title, std::string_view msg);
    }

    namespace FileDialogs {
        SE_API std::filesystem::path OpenFile(const std::string& filterName, const std::string& filter);
        SE_API std::filesystem::path SaveFile(const std::string& filterName, const std::string& filter);

        SE_API std::filesystem::path OpenFolder();
    }


    namespace Process {
        SE_API bool Run(const std::filesystem::path& filepath, const std::wstring& args);

        SE_API unsigned long GetCurrentProcessId();

        SE_API std::string GetCurrentName();
    }


    namespace Thread {
        SE_API unsigned int GetCurrentId();
    };


    struct DLLPlatformData;
    class SE_API DynamicLinkLibary {
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

    private:
        void* _GetFuncAddress(const char* name);

        DynamicLinkLibary(const DynamicLinkLibary&) = delete;
        DynamicLinkLibary(DynamicLinkLibary&&) = delete;
        DynamicLinkLibary& operator=(const DynamicLinkLibary&) = delete;
        DynamicLinkLibary& operator=(DynamicLinkLibary&&) = delete;

    private:
        DLLPlatformData* m_PlatformData = nullptr;
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