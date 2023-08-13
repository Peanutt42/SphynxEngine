#pragma once

#include "std.hpp"
#include "Core/EngineApi.hpp"
#include "Logging/Logging.hpp"

namespace Sphynx {
	class SE_API Platform {
	public:
		static bool IsDebuggerAttached();


		static void SetWorkingDirToExe();

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