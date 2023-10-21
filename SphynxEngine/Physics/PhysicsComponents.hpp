#pragma once

#include "pch.hpp"

//class btRigidBody;

namespace Sphynx::Physics {
	struct RigidbodyComponent {
		bool Dynamic = true;

		//btRigidBody* Body = nullptr;
	};

	struct BoxCollider {
		glm::vec3 HalfExtent{ 1.f, 1.f, 1.f };
	};

	struct SphereCollider {
		float Radius = 1.f;
	};
}