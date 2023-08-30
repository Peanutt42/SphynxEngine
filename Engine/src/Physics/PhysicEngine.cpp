#include "pch.hpp"
#include "PhysicEngine.hpp"
#include "Core/Engine.hpp"

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
	}

	PhysicEngine::~PhysicEngine() {
		SE_PROFILE_FUNCTION();

		ClearWorld();

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
			delete obj->getCollisionShape();
			delete obj;
		}
	}

	void PhysicEngine::Update(Scene& scene) {
		SE_PROFILE_FUNCTION();

		// remove rigidbodies that were removed
		for (int i = 0; i < m_DynamicsWorld->getNumCollisionObjects(); i++) {
			btCollisionObject* obj = m_DynamicsWorld->getCollisionObjectArray()[i];
			if (!scene.HasComponent<RigidbodyComponent>((ECS::EntityId)obj->getUserIndex())) {
				m_DynamicsWorld->removeCollisionObject(obj);
				delete obj->getCollisionShape();
				delete obj;
			}
		}
		
		// update the physic world
		for (auto [entity, transform, rb] : scene.View<ECS::TransformComponent, RigidbodyComponent>()) {
			if (rb.Body)
				_UpdateRigidbody(scene, entity, transform, rb);
		}

		
		// Add newly added RigidbodyComponent
		for (auto [entity, transform, rb] : scene.View<ECS::TransformComponent, RigidbodyComponent>()) {
			if (!rb.Body)
				_CreateRigidbody(scene, entity, transform, rb);
		}

		m_DynamicsWorld->stepSimulation(Engine::DeltaTime(), 4);

		// Sync from the physic world
		for (auto[entity, transform, rb] : scene.View<ECS::TransformComponent, RigidbodyComponent>()) {
			_SyncRigidbody(transform, rb);
		}
	}

	void PhysicEngine::_CreateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
		btCollisionShape* shape = nullptr;

		if (BoxCollider* boxCollider = scene.GetComponent<BoxCollider>(entity))
			shape = new btBoxShape(ToBtVec3(boxCollider->HalfExtent));
		else if (SphereCollider* sphereCollider = scene.GetComponent<SphereCollider>(entity))
			shape = new btSphereShape((btScalar)sphereCollider->Radius);
		else
			return;
		shape->setLocalScaling(ToBtVec3(transform.Scale));

		btTransform btTransform;
		btTransform.setIdentity();
		btTransform.setOrigin(ToBtVec3(transform.Position));
		btTransform.setRotation(ToBtQuatFromEuler(transform.Rotation));

		btScalar mass = rb.Dynamic ? 1.f : 0.f;
		btVector3 localInertia{ 0.f, 0.f, 0.f };
		if (rb.Dynamic)
			shape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* motionState = new btDefaultMotionState(btTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
		rb.Body = new btRigidBody(rbInfo);
		rb.Body->setUserIndex((int)entity);
		m_DynamicsWorld->addRigidBody(rb.Body);
	}

	void PhysicEngine::_UpdateRigidbody(Scene& scene, ECS::EntityId entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
		btTransform btTransform;
		btTransform.setIdentity();
		btTransform.setOrigin(ToBtVec3(transform.Position));
		btTransform.setRotation(ToBtQuatFromEuler(transform.Rotation));
		rb.Body->getMotionState()->setWorldTransform(btTransform);

		if (rb.Dynamic && rb.Body->getMass() == 0.f) {
			btScalar mass = 1.f;
			btVector3 localInertia;
			rb.Body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
			m_DynamicsWorld->removeRigidBody(rb.Body);
			rb.Body->setMassProps(mass, localInertia);
			rb.Body->setLinearVelocity({ 0.f, 0.f, 0.f }); // reset velo
			m_DynamicsWorld->addRigidBody(rb.Body);
			// TODO: Find a way to actually make them activate
		}
		else if (!rb.Dynamic && rb.Body->getMass() != 0.f)
			rb.Body->setMassProps(0.f, { 0.f, 0.f, 0.f });
		
		// update colliders
		if (rb.Body->getCollisionShape()->getShapeType() == BOX_SHAPE_PROXYTYPE && scene.HasComponent<BoxCollider>(entity)) {
			BoxCollider* box = scene.GetComponent<BoxCollider>(entity);
			btBoxShape* boxShape = (btBoxShape*)rb.Body->getCollisionShape();
			if (boxShape->getHalfExtentsWithMargin() != ToBtVec3(box->HalfExtent * transform.Scale)) {
				delete boxShape;
				boxShape = new btBoxShape(ToBtVec3(box->HalfExtent));
				rb.Body->setCollisionShape(boxShape);
			}
		}
		else if (rb.Body->getCollisionShape()->getShapeType() == SPHERE_SHAPE_PROXYTYPE && scene.HasComponent<SphereCollider>(entity)) {
			SphereCollider& sphere = *scene.GetComponent<SphereCollider>(entity);
			btSphereShape* sphereShape = (btSphereShape*)rb.Body->getCollisionShape();
			btScalar radius = (btScalar)sphere.Radius;
			if (sphereShape->getRadius() != radius * transform.Scale.x) {
				delete sphereShape;
				sphereShape = new btSphereShape(radius);
				rb.Body->setCollisionShape(sphereShape);
			}
#if defined(DEBUG) || defined(RELEASE)
			if (transform.Scale.x != transform.Scale.y || transform.Scale.y != transform.Scale.z)
				SE_WARN(Logging::Physics, "Non uniform scale on sphere colliders is not supported!");
#endif
		}
		else
			SE_WARN(Logging::Physics, "Invalid rigidbody collider!");

		btVector3 btScale = ToBtVec3(transform.Scale);
		if (rb.Body->getCollisionShape()->getLocalScaling() != btScale)
			rb.Body->getCollisionShape()->setLocalScaling(btScale);
	}

	void PhysicEngine::_SyncRigidbody(ECS::TransformComponent& transform, const RigidbodyComponent& rb) {
		btTransform physicTransform;
		rb.Body->getMotionState()->getWorldTransform(physicTransform);

		transform.Position = FromBtVec3(physicTransform.getOrigin());
		transform.Rotation = FromBtQuatToEuler(physicTransform.getRotation());
	}
}