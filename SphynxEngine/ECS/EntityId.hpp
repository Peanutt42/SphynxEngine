#pragma once

#include <cstdint>

namespace Sphynx::ECS {
	using EntityId = uint32_t;
	constexpr EntityId InvalidEntityId = static_cast<EntityId>(-1);
}