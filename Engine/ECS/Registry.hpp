#pragma once

#include "pch.hpp"
#include "ComponentId.hpp"
#include "Storage.hpp"
#include "EntityId.hpp"

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
				m_AliveMap.resize((size_t)m_NextEntityId * 2);
			m_AliveMap[entity] = true;
			return entity;
		}

		bool Destroy(const EntityId entity) {
			if (!IsValid(entity))
				return false;

			for (auto& [id, storage] : m_Storages)
				storage.Remove(entity);
			
			m_AliveMap[entity] = false;
			return true;
		}

		EntityId Dublicate(const EntityId entity) {
			const EntityId dublicate = Create();
			for (auto& [componentId, storage] : m_Storages) {
				if (void* srcComponent = storage.GetRaw(entity))
					storage.AddRaw(dublicate, srcComponent);
			}
			return dublicate;
		}

		template<typename T>
		T& AddComponent(const EntityId entity, const T& component = {}) {
			if (!IsValid(entity))
				return *(T*)nullptr;

			Storage& storage = CreateStorage<T>();
			return storage.Add<T>(entity, component);
		}

		template<typename T>
		bool HasComponent(const EntityId entity) const {
			if (!IsValid(entity))
				return false;

			if (const Storage* storage = FindStorage<T>())
				return storage->Has(entity);

			return false;
		}

		template<typename T>
		T* GetComponent(const EntityId entity) {
			if (!IsValid(entity))
				return nullptr;

			if (Storage* storage = FindStorage<T>())
				return storage->Get<T>(entity);

			return nullptr;
		}

		template<typename T>
		bool RemoveComponent(const EntityId entity) {
			if (!IsValid(entity))
				return false;

			if (Storage* storage = FindStorage<T>())
				storage->Remove(entity);

			return false;
		}

		template<typename T>
		T& Replace(const EntityId entity, const T& component) {
			Storage& storage = CreateStorage<T>();
			return storage.Replace<T>(entity, component);
		}

		bool IsValid(const EntityId entity) const {
			return entity < m_NextEntityId && m_AliveMap[entity];
		}

		template<typename Callback,
				 typename = std::enable_if_t<std::is_invocable_v<Callback, EntityId>>>
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
			BasicView(Registry& registry) : m_Registry(registry)
			{
				m_Storages[0] = { 0, &m_Registry.CreateStorage<T>() };

				int i = 1;
				([this, &i]() {
					m_Storages[i] = { i, &m_Registry.CreateStorage<Rest>() };
					++i;
				}(), ...);

				std::ranges::sort(m_Storages, [](const auto& left, const auto& right) {
					return left.second->Size() > right.second->Size();
				});
			}

			template<typename Callback,
				typename = std::enable_if_t<
				std::is_invocable_v<Callback, EntityId, T&, Rest&...> ||
				std::is_invocable_v<Callback, EntityId>
				>
			>
			void ForEach(const Callback& callback) {
				constexpr bool needsComponents = std::is_invocable_v<Callback, EntityId, T&, Rest&...>;

				for (EntityId entity = 0; entity < m_Registry.m_NextEntityId; ++entity) {
					std::array<void*, 1 + sizeof...(Rest)> components{};

					bool hasAllIncluded = true;
					for (const auto& [realIndex, storage] : m_Storages) {
						if constexpr (needsComponents) {
							void* componentPtr = storage->TryGetRaw(entity);
							if (!componentPtr) {
								hasAllIncluded = false;
								break;
							}
							else
								components[realIndex] = componentPtr;
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

			struct Iterator {
				Iterator(const BasicView& view, EntityId entity)
					: m_Registry(view.m_Registry), m_Storages(view.m_Storages), m_Entity(entity)
				{
					FindNext();
				}

				std::tuple<EntityId, T&, Rest&...> operator*() {
					return GetTuple<T, Rest...>(std::index_sequence_for<T, Rest...>());
				}

				bool operator==(const Iterator& other) const { return m_Entity == other.m_Entity; }

				Iterator& operator++() {
					++m_Entity;
					FindNext();
					return *this;
				}

			private:
				template<typename... Args, size_t... Indices>
				std::tuple<EntityId, T&, Rest&...> GetTuple(std::index_sequence<Indices...>) {
					return { m_Entity, *static_cast<typename std::tuple_element_t<Indices, std::tuple<Args...>>*>(m_Components[Indices])... };
				}

				void FindNext() {
					while (m_Entity < m_Registry.m_NextEntityId) {
						bool hasAllIncluded = true;
						for (const auto& [realIndex, storage] : m_Storages) {
							m_Components[realIndex] = storage->TryGetRaw(m_Entity);
							if (!m_Components[realIndex]) {
								hasAllIncluded = false;
								break;
							}
						}
						if (hasAllIncluded)
							break;

						++m_Entity;
					}
				}

				Registry& m_Registry;
				EntityId m_Entity = InvalidEntityId;
				std::array<void*, 1 + sizeof...(Rest)> m_Components;

				const std::array<std::pair<int, Storage*>, 1 + sizeof...(Rest)>& m_Storages;
			};

			Iterator begin() { return Iterator(*this, 0); }
			Iterator end() { return Iterator(*this, m_Registry.m_NextEntityId); }

		private:
			Registry& m_Registry;
			std::array<std::pair<int, Storage*>, 1 + sizeof...(Rest)> m_Storages;
		};

		template<typename T>
		struct BasicView<T> {
			BasicView(Registry& registry) : m_Registry(registry) {
				m_Storage = &m_Registry.CreateStorage<T>();
			}

			template<
				typename Callback,
				typename = std::enable_if_t<
					std::is_invocable_v<Callback, EntityId, T&> ||
					std::is_invocable_v<Callback, EntityId>
				>
			>
			void ForEach(const Callback& callback) {
				for (EntityId entity = 0; entity < m_Registry.m_NextEntityId; ++entity) {
					if constexpr (std::is_invocable_v<Callback, EntityId, T&>) {
						if (T* component = m_Storage->TryGet<T>(entity))
							callback(entity, *component);
					}
					else {
						if (m_Storage->Has(entity))
							callback(entity);
					}
				}
			}

			T& Get(const EntityId entity) {
				return *m_Storage->Get<T>(entity);
			}

			Storage::Iterator<T> begin() { return m_Storage->BeginTIterator<T>(); }
			Storage::Iterator<T> end() { return m_Storage->EndTIterator<T>(); }

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

			CopyFunc copyFunc = [](void* ptr, const void* src) {
				std::construct_at(static_cast<T*>(ptr), *static_cast<const T*>(src));
			};
			DestroyFunc destroyFunc = [](void* ptr) {
				std::destroy_at(static_cast<T*>(ptr));
			};
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