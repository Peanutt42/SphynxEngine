#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"
#include "Logging/Logging.hpp"
#include "Debug/StackTrace.hpp"

namespace Sphynx {
	class SE_API Platform {
	public:
		static bool IsDebuggerAttached();

		static void SetWorkingDirToExe();

		// context argument is only windows specific and not needed when not inside crash handeling
		static StackTrace GenerateStackTrace(void* customContext = nullptr);

		// context is only windows specific
		using ExceptionCallback = std::function<void(const std::string& reason, void* context)>;
		static void SetExceptionCallback(const ExceptionCallback& callback);

		static std::string WideToNarrow(const std::wstring& wstr) {
			std::locale loc("");
			std::string result(wstr.size(), 0);
			for (size_t i = 0; i < result.size(); i++)
				result[i] = std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr[i]);
			return result;
		}
		
		static std::wstring NarrowToWide(const std::string& str) {
			std::locale loc("");
			std::wstring result(str.size(), 0);
			for (size_t i = 0; i < result.size(); i++)
				result[i] = std::use_facet<std::ctype<wchar_t>>(loc).widen(str[i]);
			return result;
		}


		// Sets the working directory <levelsUp> times to the upper parent folder
		static void SetWorkingDirToParentFolder(size_t levelsUp) {
			std::filesystem::path currentPath = std::filesystem::current_path();
			for (size_t i = 0; i < levelsUp; i++)
				currentPath = currentPath.parent_path();
			std::filesystem::current_path(currentPath);
		}

		class SE_API MessagePrompts {
		public:
			static void Info(const std::string_view title, const std::string_view msg);
			static void Error(const std::string_view title, const std::string_view msg);
			static bool YesNo(const std::string_view title, const std::string_view msg);
		};

		class SE_API FileDialogs {
		public:
			static std::filesystem::path OpenFile(const std::string& filterName, const std::string& filter);
			static std::filesystem::path SaveFile(const std::string& filterName, const std::string& filter);

			static std::filesystem::path OpenFolder();
		};


		class SE_API Process {
		public:
			static void Run(const std::filesystem::path& filepath, const std::wstring& args);

			static unsigned long GetCurrentProcessId();

			static std::string GetCurrentName();
		};


		class SE_API Thread {
		public:
			static unsigned int GetCurrentId();
		};


		struct DLLPlatformData;
		class SE_API DynamicLinkLibary {
		public:
			DynamicLinkLibary(const std::filesystem::path& filepath);
			~DynamicLinkLibary();

			DynamicLinkLibary(const DynamicLinkLibary&) = delete;

			template<typename Func>
			Func LoadFunction(const std::string_view name) {
				Func function = (Func)_GetFuncAddress(name.data());
				SE_ASSERT(function, Logging::Scripting, "Failed to get function '{}'", name);
				return function;
			}

		private:
			void* _GetFuncAddress(const char* name);

		private:
			DLLPlatformData* m_PlatformData = nullptr;
		};
	};
}