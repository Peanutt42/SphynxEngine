#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	struct LightComponent {
		// TODO: intensity, type
		glm::vec3 Color;
	};
	
	struct CameraComponent {
		float FOV = 60.f;
		// TODO: all the other stuff
	};
}