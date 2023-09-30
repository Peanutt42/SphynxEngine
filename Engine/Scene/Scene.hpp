#pragma once

#include "pch.hpp"
#include "ECS/Registry.hpp"

namespace Sphynx {
	class Scene {
	public:
		Scene(const std::string& name);

		void SetName(const std::string& name) { m_Name = name; }
		const std::string& GetName() const { return m_Name; }

		ECS::EntityId CreateEntity(UUID uuid = UUID{});
		ECS::EntityId DublicateEntity(ECS::EntityId entity);
		void DestroyEntity(ECS::EntityId entity);
		bool IsValid(ECS::EntityId entity) { return m_Registry.IsValid(entity); }


		template<typename T>
		T& AddComponent(ECS::EntityId entity, const T& component = T{}) {
			return m_Registry.AddComponent<T>(entity, component);
		}
		template<typename T>
		bool HasComponent(ECS::EntityId entity) {
			return m_Registry.HasComponent<T>(entity);
		}
		template<typename T>
		T* GetComponent(ECS::EntityId entity) {
			return m_Registry.GetComponent<T>(entity);
		}
		template<typename T>
		void RemoveComponent(ECS::EntityId entity) {
			m_Registry.RemoveComponent<T>(entity);
		}

		template<typename... T>
		auto View() {
			return m_Registry.View<T...>();
		}
		template<typename Callback>
		void ForEach(const Callback& callback) {
			m_Registry.ForEach(callback);
		}
		ECS::Registry::Iterator begin() { return m_Registry.begin(); }
		ECS::Registry::Iterator end() { return m_Registry.end(); }

	private:
		std::string m_Name;

		ECS::Registry m_Registry;
	};
}