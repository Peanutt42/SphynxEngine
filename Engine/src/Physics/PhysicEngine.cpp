#include "pch.hpp"
#include "PhysicEngine.hpp"
#include "Core/Engine.hpp"

#include "Scene/DefaultComponents.hpp"
#include "PhysicsComponents.hpp"

#include "btUtils.hpp"

namespace Sphynx::Physics {
	PhysicEngine::PhysicEngine() {
		SE_PROFILE_FUNCTION();

		m_CollisionConfiguration = new btDefaultCollisionConfiguration();
		m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
		m_BroadphaseInterface = new btDbvtBroadphase();
		m_Solver = new btSequentialImpulseConstraintSolver();
		m_DynamicsWorld = new btDiscreteDynamicsWorld(m_Dispatcher, m_BroadphaseInterface, m_Solver, m_CollisionConfiguration);
		m_DynamicsWorld->setGravity(btVector3(0, -9.81f, 0));



		//the ground is a cube of side 100 at position y = -56.
		//the sphere will hit it at y = -6, with center at -5
		//{
		//	btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));
		//
		//	m_Shapes.push_back(groundShape);
		//
		//	btTransform groundTransform;
		//	groundTransform.setIdentity();
		//	groundTransform.setOrigin(btVector3(0, -56, 0));
		//
		//	btScalar mass(0.);
		//
		//	//rigidbody is dynamic if and only if mass is non zero, otherwise static
		//	bool isDynamic = (mass != 0.f);
		//
		//	btVector3 localInertia(0, 0, 0);
		//	if (isDynamic)
		//		groundShape->calculateLocalInertia(mass, localInertia);
		//
		//	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		//	btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		//	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		//	btRigidBody* body = new btRigidBody(rbInfo);
		//
		//	//add the body to the dynamics world
		//	m_DynamicsWorld->addRigidBody(body);
		//}
		//
		//{
		//	//create a dynamic rigidbody
		//
		//	//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		//	btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		//	m_Shapes.push_back(colShape);
		//
		//	/// Create Dynamic Objects
		//	btTransform startTransform;
		//	startTransform.setIdentity();
		//
		//	btScalar mass(1.f);
		//
		//	//rigidbody is dynamic if and only if mass is non zero, otherwise static
		//	bool isDynamic = (mass != 0.f);
		//
		//	btVector3 localInertia(0, 0, 0);
		//	if (isDynamic)
		//		colShape->calculateLocalInertia(mass, localInertia);
		//
		//	startTransform.setOrigin(btVector3(2, 10, 0));
		//
		//	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		//	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		//	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		//	btRigidBody* body = new btRigidBody(rbInfo);
		//	body->setUserIndex(m_NextBodyId++);
		//
		//	m_DynamicsWorld->addRigidBody(body);
		//}

	}

	PhysicEngine::~PhysicEngine() {
		SE_PROFILE_FUNCTION();

		ClearWorld();

		for (btCollisionShape* shape : m_Shapes)
			delete shape;
		m_Shapes.clear();

		delete m_DynamicsWorld;
		delete m_Solver;
		delete m_BroadphaseInterface;
		delete m_Dispatcher;
		delete m_CollisionConfiguration;
	}

	void PhysicEngine::ClearWorld() {
		for (int i = m_DynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
			btCollisionObject* obj = m_DynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body) {
				if (body->getMotionState())
					delete body->getMotionState();
				m_DynamicsWorld->removeRigidBody(body);
			}
			else
				m_DynamicsWorld->removeCollisionObject(obj);
			delete obj;
		}
		m_ActiveBodyIds.clear();
		m_NextBodyId = 0;
	}

	void PhysicEngine::Update(Scene& scene) {
		SE_PROFILE_FUNCTION();

		// Remove old
		{
			// remove invalid old RigidbodyComponents
			auto view = scene.View<ECS::TransformComponent, RigidbodyComponent>();
			std::vector<ECS::EntityId> toRemoveOldRigidBodyComponents;
			view.ForEach([&](ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
				if (rb.BodyIndex == -1) // newly created
					return;
				if (!m_ActiveBodyIds.contains(rb.BodyIndex))
					toRemoveOldRigidBodyComponents.push_back(entity);
			});
			for (ECS::EntityId entity : toRemoveOldRigidBodyComponents)
				scene.RemoveComponent<RigidbodyComponent>(entity);

			// remove rigidbodies that were removed
			for (int i = 0; i < m_DynamicsWorld->getNumCollisionObjects(); i++) {
				btCollisionObject* obj = m_DynamicsWorld->getCollisionObjectArray()[i];
				if (!scene.HasComponent<RigidbodyComponent>((ECS::EntityId)obj->getUserIndex()))
					m_DynamicsWorld->removeCollisionObject(obj);
			}
		}

		// Sync the physic world
		{
			auto view = scene.View<ECS::TransformComponent, RigidbodyComponent>();
			view.ForEach([&](ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
				if (rb.BodyIndex == -1) // newly created
					return;

				btTransform btTransform;
				btTransform.setIdentity();
				btTransform.setOrigin(ToBtVec3(transform.Position));
				btTransform.setRotation(ToBtQuatFromEuler(transform.Rotation));

				rb.Body->getMotionState()->setWorldTransform(btTransform);

				btVector3 btScale = ToBtVec3(transform.Scale);
				if (rb.Body->getCollisionShape()->getLocalScaling() != btScale)
					rb.Body->getCollisionShape()->setLocalScaling(btScale);
				
				if (rb.Dynamic && rb.Body->getMass() == 0.f) {
					btScalar mass = 1.f;
					btVector3 localInertia;
					rb.Body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
					rb.Body->setMassProps(mass, localInertia);
					rb.Body->setLinearVelocity({ 0.f, 0.f, 0.f }); // reset velo
				}
				else if (!rb.Dynamic && rb.Body->getMass() != 0.f)
					rb.Body->setMassProps(0.f, { 0.f, 0.f, 0.f });
				
				// TODO: check for changed collider
			});
		}

		// Add newly added RigidbodyComponent
		{
			auto view = scene.View<ECS::TransformComponent, RigidbodyComponent>();
			view.ForEach([&](ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
				if (rb.BodyIndex != -1)
					return;
				
				btCollisionShape* newShape = nullptr;

				if (BoxCollider* boxCollider = scene.GetComponent<BoxCollider>(entity))
					newShape = new btBoxShape(ToBtVec3(boxCollider->HalfExtent));
				else if (SphereCollider* sphereCollider = scene.GetComponent<SphereCollider>(entity))
					newShape = new btSphereShape((btScalar)sphereCollider->Radius);
				else
					return;
				
				newShape->setLocalScaling(ToBtVec3(transform.Scale));
				m_Shapes.push_back(newShape);

				btTransform btTransform;
				btTransform.setIdentity();
				btTransform.setOrigin(ToBtVec3(transform.Position));
				btTransform.setRotation(ToBtQuatFromEuler(transform.Rotation));

				btScalar mass = 0.f;
				if (rb.Dynamic)
					mass = 1.f;
				btVector3 localInertia{ 0.f, 0.f, 0.f };
				if (rb.Dynamic)
					newShape->calculateLocalInertia(mass, localInertia);

				btDefaultMotionState* motionState = new btDefaultMotionState(btTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, newShape, localInertia);
				rb.Body = new btRigidBody(rbInfo);
				rb.Body->setUserIndex((int)entity);
				m_DynamicsWorld->addRigidBody(rb.Body);

				rb.BodyIndex = m_NextBodyId++;
				m_ActiveBodyIds.insert(rb.BodyIndex);
			});
		}

		m_DynamicsWorld->stepSimulation(Engine::DeltaTime(), 4);

		// Sync from the physic world
		{
			auto view = scene.View<ECS::TransformComponent, RigidbodyComponent>();
			view.ForEach([&](ECS::EntityId entity, ECS::TransformComponent& transform, const RigidbodyComponent& rb) {
				btTransform btTransform;
				rb.Body->getMotionState()->getWorldTransform(btTransform);
				
				transform.Position = FromBtVec3(btTransform.getOrigin());
				transform.Rotation = FromBtQuatToEuler(btTransform.getRotation());
				transform.Scale = FromBtVec3(rb.Body->getCollisionShape()->getLocalScaling());
			});
		}
	}
}