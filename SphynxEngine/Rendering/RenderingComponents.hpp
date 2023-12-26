#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	struct MeshComponent {
		glm::vec3 Albedo;
		float Metalic;
		float Roughness;
	};

	struct LightComponent {
		glm::vec3 Color;
	};
}