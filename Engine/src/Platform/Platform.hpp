#pragma once

#include "std.hpp"

namespace Sphynx {
	class Platform {
	public:
		static bool IsDebuggerAttached();

		class MessagePrompts {
		public:
			static void Info(const std::string_view title, const std::string_view msg);
			static void Error(const std::string_view title, const std::string_view msg);
			static bool YesNo(const std::string_view title, const std::string_view msg);
		};

		class FileDialogs {
		public:
			static std::filesystem::path OpenFile(const wchar_t* filter);
			static std::filesystem::path SaveFile(const wchar_t* filter);

			static std::filesystem::path OpenFolder();
		};
	};
}