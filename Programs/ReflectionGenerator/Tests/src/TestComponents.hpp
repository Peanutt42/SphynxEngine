#pragma once

#include "../../ScriptingDefines.hpp"

Component();
struct NoNamespaceComponent {
	int AInteger = 0;
};

namespace ANamespace {
	Component();
	struct ComponentWithNamespace {
		float AFloat = 0.f;
	};

	Component();
	struct NestedComponent {
		struct NestedStruct {
			int integer = 0;
		} Nested;
	};
}