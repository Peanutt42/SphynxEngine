#pragma once

#include <vector>
#include <string>

namespace Sphynx::Scripting {
	struct VariableReflectionInfo {
		std::string Type;
		std::string Name;

		// Used in runtime
		size_t Offset = 0;
		size_t Size = 0;
		void(*CopyConstructor)(void*, void*) = nullptr;
	};

	struct ComponentReflectionInfo {
		size_t TypeID = 0;
		std::string FullName;
		std::vector<VariableReflectionInfo> Variables;

		// Used in runtime
		size_t Size = 0;
		void(*Constructor)(void*) = nullptr;
		void(*Destructor)(void*) = nullptr;
		void(*CopyConstructor)(void*, void*) = nullptr;
	};

	struct ConfigReflectionInfo : public ComponentReflectionInfo {};

	struct SystemReflectionInfo {
		size_t TypeID = 0;
		std::string FullName;

		// Used in runtime
		void(*Update)(void* scene) = nullptr;
	};
}