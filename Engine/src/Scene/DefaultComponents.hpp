#pragma once

#include "pch.hpp"

namespace Sphynx::ECS {
	struct NameComponent {
		std::string Name;
	};

	struct UUIDComponent {
		UUID uuid = 0;
	};

	struct TransformComponent : public Transform { };
}