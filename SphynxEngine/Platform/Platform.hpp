#pragma once

#include "std.hpp"
#include "Logging/Logging.hpp"

namespace Sphynx::Platform {
    inline static std::thread::id s_MainThreadId = std::this_thread::get_id();

    SE_API bool IsDebuggerAttached();

    SE_API bool ConsoleSupportsColor();

    SE_API void SetWorkingDirToExe();

    // context is only for windows stacktracing!
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
    }


    namespace Process {
        SE_API bool Run(const std::filesystem::path& filepath, const std::wstring& args);

        SE_API unsigned long GetCurrentProcessId();

        SE_API std::string GetCurrentName();
    }


    class SE_API DynamicLinkLibrary {
    public:
        DynamicLinkLibrary() = default;
        DynamicLinkLibrary(const std::filesystem::path& filepath) { m_Open = Open(filepath); }
        ~DynamicLinkLibrary() { Close(); }

        bool Open(const std::filesystem::path& filepath) {
            if (m_PlatformHandle)
                _Close();
            m_Filepath = filepath;
            m_Open = _Open();
            return m_Open;
        }
        void Close() {
            if (m_PlatformHandle) {
                _Close();
                m_FunctionMap.clear();
            }
        }

        template<typename Func>
        std::optional<Func> LoadFunction(std::string_view name) {
            if (!m_Open) {
                SE_ERR("Tried to load a function with a dll that isn't opened yet!");
                return std::nullopt;
            }
            auto findCachedFunction = m_FunctionMap.find(name);
            if (findCachedFunction != m_FunctionMap.end())
                return (Func)findCachedFunction->second;

            Func function = (Func)_GetFuncAddress(name.data());
            if (!function) {
                SE_ERR("Failed to find function '{}' in dll '{}'", name, m_Filepath.string());
                return std::nullopt;
            }
            m_FunctionMap[name] = function;
            return function;
        }

        template<typename... Args>
        void Invoke(std::string_view name, Args&&... args) {
            using Func = void(*)(Args... args);
            if (auto optFunction = LoadFunction<Func>(name))
                (*optFunction)(std::forward<Args>(args)...);
        }
        template<typename TResult, typename... Args>
        TResult Invoke(std::string_view name, Args&&... args) {
            using Func = TResult(*)(Args... args);
            if (auto optFunction = LoadFunction<Func>(name))
                return (*optFunction)(std::forward<Args>(args)...);
            else
                return TResult{};
        }

        bool IsOpen() const { return m_Open; }

        const std::filesystem::path& GetFilepath() const { return m_Filepath; }

        static bool IsDLL(const std::filesystem::path& filepath);
        // returns extension with the . on the left: example: ".dll"
        static const char* DLLExtension();

    private:
        bool _Open();
        void _Close();
        void* _GetFuncAddress(const char* name);

        DynamicLinkLibrary(const DynamicLinkLibrary&) = delete;
        DynamicLinkLibrary(DynamicLinkLibrary&&) = delete;
        DynamicLinkLibrary& operator=(const DynamicLinkLibrary&) = delete;
        DynamicLinkLibrary& operator=(DynamicLinkLibrary&&) = delete;

    private:
        void* m_PlatformHandle = nullptr;
        bool m_Open = false;
        std::filesystem::path m_Filepath;
        std::unordered_map<std::string_view, void*> m_FunctionMap;
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