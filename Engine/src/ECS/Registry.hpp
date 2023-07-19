#pragma once

#include "ComponentId.hpp"
#include "Storage.hpp"
#include "EntityId.hpp"
#include <memory>
#include <unordered_map>
#include <functional>
#include <array>
#include <cassert>

namespace Sphynx::ECS {
	class Registry {
	public:
		Registry() {

		}

		~Registry() {
			Clear();
		}

		void Clear() {
			m_Storages.clear();
		}

		template<typename T>
		void Clear() {
			if (m_Storages.contains(GetComponentId<T>()))
				m_Storages.erase(GetComponentId<T>());
		}

		EntityId Create() {
			const EntityId entity = m_NextEntityId++;
			if (m_AliveMap.size() <= m_NextEntityId)
				m_AliveMap.resize(m_NextEntityId * 2);
			m_AliveMap[entity] = true;
			return entity;
		}

		bool Destroy(const EntityId entity) {
			if (!IsValid(entity))
				return false;

			bool removeSuccessful = true;
			for (auto& [id, storage] : m_Storages) {
				if (!storage.Remove(entity))
					removeSuccessful = false;
			}
			m_AliveMap[entity] = false;
			return removeSuccessful;
		}

		EntityId Dublicate(const EntityId entity) {
			const EntityId dublicate = Create();
			for (auto& [componentId, storage] : m_Storages) {
				void* srcComponent = nullptr;
				if (storage.TryGet(entity, &srcComponent))
					storage.AddRaw(dublicate, srcComponent);
			}
			return dublicate;
		}

		template<typename T>
		T& AddComponent(const EntityId entity, const T& component) {
			if (!IsValid(entity))
				return *(T*)nullptr;

			Storage& storage = CreateStorage<T>();
			return storage.Add<T>(entity, component);
		}

		template<typename T>
		bool HasComponent(const EntityId entity) const {
			if (!IsValid(entity))
				return false;

			const Storage* storage = FindStorage<T>();
			if (!storage)
				return false;

			return storage->Has(entity);
		}

		template<typename T>
		T& GetComponent(const EntityId entity) {
			if (!IsValid(entity))
				return *(T*)nullptr;

			Storage& storage = CreateStorage<T>();
			return storage.Get<T>(entity);
		}

		template<typename T>
		bool RemoveComponent(const EntityId entity) {
			if (!IsValid(entity))
				return false;

			Storage& storage = CreateStorage<T>();
			return storage.Remove(entity);
		}

		template<typename T, typename... Args>
		T& AddOrReplace(const EntityId entity, Args&&... args) {
			Storage& storage = CreateStorage<T>();
			T* ptr = nullptr;
			if (storage.TryGet(entity, &ptr)) {
				std::construct_at(ptr, std::forward<Args>(args)...);
				return *ptr;
			}
			else
				return storage.Add<T>(entity);
		}

		bool IsValid(const EntityId entity) const {
			return entity < m_NextEntityId && m_AliveMap[entity];
		}

		template<typename Callback>
		void ForEach(const Callback& callback) {
			for (EntityId entity = 0; entity < m_NextEntityId; ++entity) {
				if (IsValid(entity))
					callback(entity);
			}
		}

		// Invokes Callbacks that take the Component& component... as arguments
		template<typename Func, typename... Args, size_t... Indices>
		static void CallFuncWithComponents(const Func& func, EntityId entity, const std::array<void*, sizeof...(Args)>& args, std::index_sequence<Indices...>) {
			func(entity, *static_cast<typename std::tuple_element_t<Indices, std::tuple<Args...>>*>(args[Indices])...);
		}

		template<typename T, typename... Rest>
		struct BasicView {
			BasicView(Registry& registry) : m_Registry(registry) {	}

			template<typename Callback>
			void ForEach(const Callback& callback) {
				std::array<std::pair<int, Storage*>, 1 + sizeof...(Rest)> storages{};
				storages[0] = { 0, &m_Registry.CreateStorage<T>() };

				int i = 1;
				([this, &storages, &i]() {
					storages[i] = { i, &m_Registry.CreateStorage<Rest>() };
					++i;
					}(), ...);

				std::ranges::sort(storages, [](const auto& left, const auto& right) {
					return left.second->Size() > right.second->Size();
					});

				constexpr bool needsComponents = std::is_invocable_v<Callback, EntityId, T&, Rest&...>;

				for (EntityId entity = 0; entity < m_Registry.m_NextEntityId; ++entity) {
					std::array<void*, 1 + sizeof...(Rest)> components{};

					bool hasAllIncluded = true;
					for (const auto& [realIndex, storage] : storages) {
						if constexpr (needsComponents) {
							if (!storage->TryGet(entity, &components[realIndex])) {
								hasAllIncluded = false;
								break;
							}
						}
						else {
							if (!storage->Has(entity)) {
								hasAllIncluded = false;
								break;
							}
						}
					}
					if (!hasAllIncluded)
						continue;

					if constexpr (needsComponents) {
						CallFuncWithComponents<Callback, T, Rest...>(callback, entity, components, std::index_sequence_for<T, Rest...>());
					}
					else {
						callback(entity);
					}
				}
			}

		private:
			Registry& m_Registry;
		};

		template<typename T>
		struct BasicView<T> {
			BasicView(Registry& registry) : m_Registry(registry) {
				m_Storage = &m_Registry.CreateStorage<T>();
			}

			template<typename Callback>
			void ForEach(const Callback& callback) {
				for (EntityId entity = 0; entity < m_Registry.m_NextEntityId; ++entity) {
					if constexpr (std::is_invocable_v<Callback, EntityId, T&>) {
						void* component = nullptr;
						if (m_Storage->TryGet(entity, &component))
							callback(entity, *static_cast<T*>(component));
					}
					else {
						if (m_Storage->Has(entity))
							callback(entity);
					}
				}
			}

			T& Get(const EntityId entity) {
				return m_Storage->Get<T>(entity);
			}

		private:
			Registry& m_Registry;
			Storage* m_Storage = nullptr;
		};

		template<typename... T>
		BasicView<T...> View() {
			return BasicView<T...>(*this);
		}

		template<typename T>
		const Storage* FindStorage() const {
			constexpr ComponentId id = GetComponentId<T>();
			const auto findStorage = m_Storages.find(id);
			if (findStorage != m_Storages.end())
				return &findStorage->second;
			return nullptr;
		}

		template<typename T>
		Storage* FindStorage() {
			constexpr ComponentId id = GetComponentId<T>();
			const auto findStorage = m_Storages.find(id);
			if (findStorage != m_Storages.end())
				return &findStorage->second;
			return nullptr;
		}

		template<typename T>
		Storage& CreateStorage() {
			constexpr ComponentId id = GetComponentId<T>();
			Storage* storage = FindStorage<T>();
			if (storage)
				return *storage;

			CopyFunc copyFunc = [](const void* src, void* dst) { std::construct_at(static_cast<T*>(dst), *static_cast<const T*>(src)); };
			DestroyFunc destroyFunc = [](void* ptr) { std::destroy_at(static_cast<T*>(ptr)); };
			m_Storages[id] = Storage(
				sizeof(T),
				copyFunc,
				destroyFunc
			);
			return m_Storages.at(id);
		}

	private:
		EntityId m_NextEntityId = 0;
		std::vector<bool> m_AliveMap;
		std::unordered_map<ComponentId, Storage> m_Storages;
	};
}