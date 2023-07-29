#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "DefaultComponents.hpp"

namespace Sphynx {
	Scene::Scene(const std::string& name)
		: m_Name(name)
	{

	}

	ECS::EntityId Scene::CreateEntity(UUID uuid) {
		ECS::EntityId entity = m_Registry.Create();

		m_Registry.AddComponent(entity, ECS::NameComponent{"Noname"});
		m_Registry.AddComponent(entity, ECS::UUIDComponent{ uuid });
		m_Registry.AddComponent(entity, ECS::TransformComponent{});

		return entity;
	}
	
	ECS::EntityId Scene::DublicateEntity(ECS::EntityId entity) {
		return m_Registry.Dublicate(entity);
	}

	void Scene::DestroyEntity(ECS::EntityId entity) {
		m_Registry.Destroy(entity);
	}
}