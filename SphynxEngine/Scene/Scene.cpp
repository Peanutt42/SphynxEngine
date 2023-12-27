#include "pch.hpp"
#include "Scene/Scene.hpp"
#include "AllComponents.hpp"

namespace Sphynx {
	Scene::Scene(const std::string& name)
		: m_Name(name) {}

	template<typename Component>
	void CopyComponent(entt::registry& registry, entt::entity dst, entt::entity src) {
		if (Component* component = registry.try_get<Component>(src))
			registry.emplace_or_replace<Component>(dst, *component);
	}
	template<typename... Component>
	void CopyComponents(ECS::ComponentList<Component...>, entt::registry& registry, entt::entity dst, entt::entity src) {
		(CopyComponent<Component>(registry, dst, src), ...);
	}

	template<typename Component>
	void CopyComponent(entt::registry& dst, const entt::registry& src) {
		auto view = src.view<Component>();
		for (auto entity : view)
			dst.emplace_or_replace<Component>(entity, view.template get<Component>(entity));
	}
	template<typename... Component>
	void CopyComponents(ECS::ComponentList<Component...>, entt::registry& dst, const entt::registry& src) {
		(CopyComponent<Component>(dst, src), ...);
	}

	Scene::Scene(const Scene& other) {
		// m_Name = other.m_Name;
		m_SystemsActiveMap = other.m_SystemsActiveMap;

		m_Registry.clear();
		auto otherData = other.m_Registry.storage<entt::entity>()->data();
		auto otherSize = other.m_Registry.storage<entt::entity>()->size();
		auto otherInUse = other.m_Registry.storage<entt::entity>()->in_use();
		m_Registry.storage<entt::entity>().push(otherData, otherData + otherSize);
		m_Registry.storage<entt::entity>().in_use(otherInUse);
		CopyComponents(ECS::AllComponents{}, m_Registry, other.m_Registry);
	}

	entt::entity Scene::CreateEntity(UUID uuid) {
		entt::entity entity = m_Registry.create();

		m_Registry.emplace<ECS::NameComponent>(entity, "Noname");
		m_Registry.emplace<ECS::UUIDComponent>(entity, uuid);
		m_Registry.emplace<ECS::TransformComponent>(entity);

		return entity;
	}

	entt::entity Scene::DublicateEntity(entt::entity entity) {
		entt::entity dublicate = CreateEntity();
		CopyComponents(ECS::AllComponents{}, m_Registry, dublicate, entity);
		return dublicate;
	}
	
	void Scene::DestroyEntity(entt::entity entity) {
		m_Registry.destroy(entity);
	}


	void Scene::ActivateSystem(std::string_view name) {
		m_SystemsActiveMap[name] = true;
	}

	void Scene::DeactivateSystem(std::string_view name) {
		m_SystemsActiveMap[name] = false;
	}
	
	bool Scene::IsSystemActive(std::string_view name) {
		auto findSystem = m_SystemsActiveMap.find(name);
		if (findSystem == m_SystemsActiveMap.end())
			return false;
		return findSystem->second;
	}


	entt::entity Scene::GetActiveCameraEntity() {
		for (auto [entity, camera] : m_Registry.view<Rendering::CameraComponent>().each()) {
			if (camera.Active)
				return entity;
		}
		return entt::null;
	}
}