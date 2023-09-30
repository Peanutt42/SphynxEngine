#pragma once

#include <vector>
#include <string>

namespace Sphynx {
	class Scene;
}

namespace Sphynx::Scripting {
	struct VariableReflectionInfo {
		std::string Type;
		std::string Name;

		bool IsEquivalent(const VariableReflectionInfo& other) const {
			return Type == other.Type && Name == other.Name;
		}

		// Used in runtime
		size_t Offset = 0;
		size_t Size = 0;
		void(*CopyConstructor)(void*, void*) = nullptr;
	};

	struct ComponentReflectionInfo {
		size_t TypeID = 0;
		std::string FullName;
		std::vector<VariableReflectionInfo> Variables;

		bool IsEquivalent(const ComponentReflectionInfo& other) const {
			if (TypeID != other.TypeID || FullName != other.FullName)
				return false;

			for (const auto& var : Variables) {
				bool foundEquivalent = false;
				for (const auto& otherVar : other.Variables) {
					if (var.IsEquivalent(otherVar)) {
						foundEquivalent = true;
						break;
					}
				}
				if (!foundEquivalent)
					return false;
			}

			return true;
		}

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
		void(*Update)(Sphynx::Scene& scene) = nullptr;
	};
}