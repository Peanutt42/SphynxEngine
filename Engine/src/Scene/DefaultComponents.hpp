#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::ECS {
	struct NameComponent {
		std::string Name;
	};

	struct UUIDComponent {
		UUID uuid = 0;
	};
}