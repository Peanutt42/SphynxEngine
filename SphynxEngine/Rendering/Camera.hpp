#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	// safe, it will return 1920 / 1080 if the original aspect was nan
	inline static float GetAspect(float x, float y) {
		float aspect = x / y;
		if (std::isnan(aspect))
			return 1920.f / 1080.f;
		return aspect;
	}

	struct Camera {
		glm::vec3 Position{ 0.f, 0.f, 10.f };
		glm::vec3 Rotation{ 0.f, 0.f, 0.f };

		float Fov = 60.f;
		float NearPlane = 0.1f, FarPlane = 1000.f;

		glm::vec3 GetForward() const {
			return glm::quat(Rotation) * glm::vec3(0.f, 0.f, -1.f);
		}
		glm::vec3 GetRight() const {
			return glm::normalize(glm::cross(GetForward(), { 0.f, 1.f, 0.f }));
		}
		glm::vec3 GetUp() const {
			return glm::normalize(glm::cross(GetRight(), GetForward()));
		}

		glm::mat4 GetPerspective(float aspect) const {
			glm::mat4 perspective = glm::perspective(glm::radians(Fov), aspect, NearPlane, FarPlane);
			return perspective;
		}

		glm::mat4 GetView() const {
			return glm::inverse(
				glm::translate(glm::mat4(1.f), Position) * glm::toMat4(glm::quat(Rotation))
			);
		}
	};
}