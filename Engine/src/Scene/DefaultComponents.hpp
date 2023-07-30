#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::ECS {
	struct NameComponent {
		std::string Name;
	};

	struct UUIDComponent {
		UUID uuid = 0;
	};

	struct TransformComponent {
		glm::vec3 Position{ 0.f, 0.f, 0.f };
		glm::vec3 Rotation{ 0.f, 0.f, 0.f };
		glm::vec3 Scale{ 1.f, 1.f, 1.f };
	};
}