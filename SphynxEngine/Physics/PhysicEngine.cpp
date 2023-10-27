#include "pch.hpp"
#include "PhysicEngine.hpp"
#include "Core/Engine.hpp"
#include "Profiling/Profiling.hpp"

#include "btUtils.hpp"

namespace Sphynx::Physics {
	btCollisionConfiguration* s_CollisionConfiguration = nullptr;
	btCollisionDispatcher* s_Dispatcher = nullptr;
	btBroadphaseInterface* s_BroadphaseInterface = nullptr;
	btSequentialImpulseConstraintSolver* s_Solver = nullptr;
	btDiscreteDynamicsWorld* s_DynamicsWorld = nullptr;


	void PhysicEngine::Init() {
		SE_PROFILE_FUNCTION();

		s_CollisionConfiguration = new btDefaultCollisionConfiguration();
		s_Dispatcher = new btCollisionDispatcher(s_CollisionConfiguration);
		s_BroadphaseInterface = new btDbvtBroadphase();
		s_Solver = new btSequentialImpulseConstraintSolver();
		s_DynamicsWorld = new btDiscreteDynamicsWorld(s_Dispatcher, s_BroadphaseInterface, s_Solver, s_CollisionConfiguration);
		s_DynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	}

	void PhysicEngine::Shutdown() {
		SE_PROFILE_FUNCTION();

		ClearWorld();

		delete s_DynamicsWorld;
		delete s_Solver;
		delete s_BroadphaseInterface;
		delete s_Dispatcher;
		delete s_CollisionConfiguration;
	}

	void PhysicEngine::ClearWorld() {
		for (int i = s_DynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
			btCollisionObject* obj = s_DynamicsWorld->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);
			if (body) {
				if (body->getMotionState())
					delete body->getMotionState();
				s_DynamicsWorld->removeRigidBody(body);
			}
			else
				s_DynamicsWorld->removeCollisionObject(obj);
			delete obj->getCollisionShape();
			delete obj;
		}
	}

	void PhysicEngine::Update(Scene& scene) {
		SE_PROFILE_FUNCTION();
#if 0
		// remove rigidbodies that were removed
		for (int i = 0; i < s_DynamicsWorld->getNumCollisionObjects(); i++) {
			btCollisionObject* obj = s_DynamicsWorld->getCollisionObjectArray()[i];
			if (!scene.HasComponent<RigidbodyComponent>((ECS::EntityId)obj->getUserIndex())) {
				s_DynamicsWorld->removeCollisionObject(obj);
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

		s_DynamicsWorld->stepSimulation(Engine::DeltaTime(), 0);

		// Sync from the physic world
		for (auto[entity, transform, rb] : scene.View<ECS::TransformComponent, RigidbodyComponent>()) {
			_SyncRigidbody(transform, rb);
		}
#endif
	}

	void PhysicEngine::_CreateRigidbody(Scene& scene, entt::entity entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
#if 0
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

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, nullptr, shape, localInertia);
		rb.Body = new btRigidBody(rbInfo);
		rb.Body->setWorldTransform(btTransform);
		rb.Body->setUserIndex((int)entity);
		s_DynamicsWorld->addRigidBody(rb.Body);
#endif
	}

	void PhysicEngine::_UpdateRigidbody(Scene& scene, entt::entity entity, const ECS::TransformComponent& transform, RigidbodyComponent& rb) {
#if 0
		bool changed = false;

		const btTransform& physicTransform = rb.Body->getCenterOfMassTransform();

		if (transform.Position != FromBtVec3(physicTransform.getOrigin()) ||
			transform.Rotation != FromBtQuatToEuler(physicTransform.getRotation()))
		{
			changed = true;

			btTransform btTransform;
			btTransform.setIdentity();
			btTransform.setOrigin(ToBtVec3(transform.Position));
			btTransform.setRotation(ToBtQuatFromEuler(transform.Rotation));
			rb.Body->setWorldTransform(btTransform);
		}

		if (rb.Dynamic && rb.Body->getMass() == 0.f) {
			changed = true;

			btScalar mass = 1.f;
			btVector3 localInertia;
			rb.Body->getCollisionShape()->calculateLocalInertia(mass, localInertia);
			rb.Body->setMassProps(mass, localInertia);
			rb.Body->setLinearVelocity({ 0.f, 0.f, 0.f });
			rb.Body->setAngularVelocity({ 0.f, 0.f, 0.f });
			rb.Body->clearForces();
		}
		else if (!rb.Dynamic && rb.Body->getMass() != 0.f) {
			changed = true;
			rb.Body->setMassProps(0.f, { 0.f, 0.f, 0.f });
			rb.Body->setLinearVelocity({ 0.f, 0.f, 0.f });
			rb.Body->setAngularVelocity({ 0.f, 0.f, 0.f });
			rb.Body->clearForces();
		}
		
		// update colliders
		if (rb.Body->getCollisionShape()->getShapeType() == BOX_SHAPE_PROXYTYPE && scene.HasComponent<BoxCollider>(entity)) {
			BoxCollider* box = scene.GetComponent<BoxCollider>(entity);
			btBoxShape* boxShape = (btBoxShape*)rb.Body->getCollisionShape();
			if (boxShape->getHalfExtentsWithMargin() != ToBtVec3(box->HalfExtent * transform.Scale)) {
				changed = true;

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
				changed = true;

				delete sphereShape;
				sphereShape = new btSphereShape(radius);
				rb.Body->setCollisionShape(sphereShape);
			}
#if defined(DEBUG) || defined(DEVELOPMENT)
			if (transform.Scale.x != transform.Scale.y || transform.Scale.y != transform.Scale.z)
				SE_WARN(Logging::Physics, "Non uniform scale on sphere colliders is not supported!");
#endif
		}
		else
			SE_WARN(Logging::Physics, "Invalid rigidbody collider!");

		btVector3 btScale = ToBtVec3(transform.Scale);
		if (rb.Body->getCollisionShape()->getLocalScaling() != btScale) {
			changed = true;
			rb.Body->getCollisionShape()->setLocalScaling(btScale);
		}

		if (changed)
			rb.Body->activate(true);
#endif
	}

	void PhysicEngine::_SyncRigidbody(ECS::TransformComponent& transform, const RigidbodyComponent& rb) {
#if 0
		const btTransform& actualTransform = rb.Body->getCenterOfMassTransform();
		transform.Position = FromBtVec3(actualTransform.getOrigin());
		transform.Rotation = FromBtQuatToEuler(actualTransform.getRotation());
#endif
	}
}