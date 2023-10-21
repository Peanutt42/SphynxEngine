#pragma once

#include "pch.hpp"
#include <entt/entt.hpp>

namespace Sphynx {
	class SE_API Scene {
	public:
		Scene(const std::string& name);

		Scene(const Scene& other);
		Scene& operator=(const Scene& other);

		void SetName(const std::string& name) { m_Name = name; }
		const std::string& GetName() const { return m_Name; }

		entt::entity CreateEntity(UUID uuid = UUID{});
		entt::entity DublicateEntity(entt::entity entity);
		void DestroyEntity(entt::entity entity);
		bool IsValid(entt::entity entity) { return m_Registry.valid(entity); }


		template<typename T>
		T& AddComponent(entt::entity entity, const T& component = T{}) {
			return m_Registry.emplace<T>(entity, component);
		}
		template<typename T>
		bool HasComponent(entt::entity entity) {
			return m_Registry.any_of<T>(entity);
		}
		template<typename T>
		T* GetComponent(entt::entity entity) {
			return m_Registry.try_get<T>(entity);
		}
		template<typename T>
		void RemoveComponent(entt::entity entity) {
			m_Registry.remove<T>(entity);
		}

		template<typename... T>
		auto View() {
			return m_Registry.view<T...>();
		}
		template<typename Callback>
		void ForEach(const Callback& callback) {
			for (auto[entity] : m_Registry.storage<entt::entity>().each()) {
				callback(entity);
			}
		}

		void ActivateSystem(std::string_view name);
		void DeactivateSystem(std::string_view name);
		bool IsSystemActive(std::string_view name);
		const std::unordered_map<std::string_view, bool>& GetSystemsActiveMap() const { return m_SystemsActiveMap; }

	private:
		std::string m_Name;

		entt::registry m_Registry;

		std::unordered_map<std::string_view, bool> m_SystemsActiveMap;
	};
}