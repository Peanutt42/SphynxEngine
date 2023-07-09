#pragma once

#include <string>
#include <vector>

struct StackTraceEntry {
	std::string FunctionName;
	bool HasSource = false;
	std::string SourceFile;
	size_t SourceLine = 0;
};
struct StackTrace {
	std::vector<StackTraceEntry> Entries;
};