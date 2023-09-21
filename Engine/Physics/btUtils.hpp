#pragma once

#include "pch.hpp"

#define BT_THREADSAFE 1
#define BT_USE_DOUBLE_PRECISION
#pragma warning (push)
#pragma warning (disable: 4127) // expression could be constexpr
#include "btBulletDynamicsCommon.h"
#pragma warning (pop)

namespace Sphynx::Physics {
	inline glm::vec3 FromBtVec3(const btVector3& btVec3) {
		return {
			(float)btVec3.getX(),
			(float)btVec3.getY(),
			(float)btVec3.getZ()
		};
	}

	inline btVector3 ToBtVec3(const glm::vec3& vec3) {
		return {
			(btScalar)vec3.x,
			(btScalar)vec3.y,
			(btScalar)vec3.z
		};
	}

	inline glm::vec3 FromBtQuatToEuler(const btQuaternion& btQuat) {
		btScalar yawZ, pitchY, rollX;
		btQuat.getEulerZYX(yawZ, pitchY, rollX);
		return {
			(float)rollX,
			(float)pitchY,
			(float)yawZ
		};
	}

	inline btQuaternion ToBtQuatFromEuler(const glm::vec3& euler) {
		return {
			(btScalar)euler.y,
			(btScalar)euler.x,
			(btScalar)euler.z
		};
	}
}