#pragma once

#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/DefaultComponents.hpp"
#include "PhysicsComponents.hpp"

class btCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btCollisionShape;

namespace Sphynx::Physics {
	class SE_API PhysicEngine {
	public:
		PhysicEngine();
		~PhysicEngine();

		void Update(Scene& scene);

		void ClearWorld();
		
	private:
		void _CreateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb);
		void _UpdateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb);
		void _SyncRigidbody(ECS::TransformComponent& transform, const RigidbodyComponent& rb);

	private:
		btCollisionConfiguration* m_CollisionConfiguration = nullptr;
		btCollisionDispatcher* m_Dispatcher = nullptr;
		btBroadphaseInterface* m_BroadphaseInterface = nullptr;
		btSequentialImpulseConstraintSolver* m_Solver = nullptr;
		btDiscreteDynamicsWorld* m_DynamicsWorld = nullptr;
	};
}