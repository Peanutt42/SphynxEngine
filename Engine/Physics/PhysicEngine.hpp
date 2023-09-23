#pragma once

#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "Scene/DefaultComponents.hpp"
#include "PhysicsComponents.hpp"

namespace Sphynx::Physics {
	class SE_API PhysicEngine {
	public:
		static void Init();
		static void Shutdown();

		static void Update(Scene& scene);

		// TODO: Physic Scene
		static void ClearWorld();
		
	private:
		static void _CreateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb);
		static void _UpdateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb);
		static void _SyncRigidbody(ECS::TransformComponent& transform, const RigidbodyComponent& rb);
	};
}