#pragma once

#include "pch.hpp"
#define GLM_FORCE_SILENT_WARNINGS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Sphynx {
	struct SE_API Transform {
		glm::vec3 Position{ 0.f, 0.f, 0.f };
		glm::vec3 Rotation{ 0.f, 0.f, 0.f };
		glm::vec3 Scale{ 1.f, 1.f, 1.f };

		glm::mat4 GetModelMatrix() const {
			return glm::translate(glm::mat4(1.f), Position) *
				glm::toMat4(glm::quat(Rotation)) *
				glm::scale(glm::mat4(1.f), Scale);
		}
	};
}