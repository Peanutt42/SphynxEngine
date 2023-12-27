#pragma once

#include "pch.hpp"
#include "Camera.hpp"

namespace Sphynx::Rendering {
	struct MeshComponent {
		glm::vec3 Albedo;
		float Metalic;
		float Roughness;
	};

	struct LightComponent {
		glm::vec3 Color;
	};

	struct CameraComponent {
		bool Active = true;

		float FOV = 60.f;
		float NearPlane = 0.1f;
		float FarPlane = 1000.f;

		Camera ToCamera(const Transform& transform) const {
			return Camera{
				.Position = transform.Position,
				.Rotation = transform.Rotation,
				.Fov = FOV,
				.NearPlane = NearPlane,
				.FarPlane = FarPlane,
			};
		}
	};
}