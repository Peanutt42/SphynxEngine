#pragma once

#include <string>
#include <vector>

namespace Sphynx {
	struct StackTraceEntry {
		std::string FunctionName;

		bool HasModule = false;
		std::string ModuleName;
		
		bool HasSource = false;
		std::string SourceFile;
		size_t SourceLine = 0;
	};

	using StackTrace = std::vector<StackTraceEntry>;
}