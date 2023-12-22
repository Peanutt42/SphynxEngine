#pragma once

#include "pch.hpp"
#include "Camera.hpp"

namespace Sphynx::Rendering {
	struct LightComponent {
		// TODO: intensity, type
		glm::vec3 Color;
	};
	
	struct CameraComponent {
		float FOV = 60.f;
		float NearPlane = 0.1f, FarPlane = 1000.f;

		Camera ToCamera(const ECS::TransformComponent& transform) const {
			return {
				.Position = transform.Position,
				.Rotation = transform.Rotation,
				.Fov = FOV,
				.NearPlane = NearPlane,
				.FarPlane = FarPlane
			};
		}
	};

	struct MeshComponent {
		int __buffer; // just for a non 0 sized component
	};
}