#pragma once

#include "pch.hpp"
#include "Scene/Scene.hpp"

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
		btCollisionConfiguration* m_CollisionConfiguration = nullptr;
		btCollisionDispatcher* m_Dispatcher = nullptr;
		btBroadphaseInterface* m_BroadphaseInterface = nullptr;
		btSequentialImpulseConstraintSolver* m_Solver = nullptr;
		btDiscreteDynamicsWorld* m_DynamicsWorld = nullptr;
		std::vector<btCollisionShape*> m_Shapes;

		int m_NextBodyId = 0;
		std::set<int> m_ActiveBodyIds;
	};
}