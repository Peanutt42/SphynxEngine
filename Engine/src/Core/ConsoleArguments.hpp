#pragma once

#include "CoreInclude.hpp"

namespace Sphynx {
	struct ConsoleArguments {
		ConsoleArguments() = default;

		ConsoleArguments(const int argc, const char** argv) : m_Argc(argc), m_Argv(argv) {
			for (int i = 1; i < argc; i++)
				m_Arguments.insert(std::string_view(argv[i]));
		}

		std::string_view GetArgument(int index) {
			if (index < m_Argc)
				return m_Argv[index];
			else
				return "";
		}

		bool HasArgument(const std::string_view arg) const {
			return m_Arguments.contains(arg);
		}

	private:
		const int m_Argc = 0;
		const char** m_Argv = nullptr;
		std::set<std::string_view> m_Arguments;
	};
}