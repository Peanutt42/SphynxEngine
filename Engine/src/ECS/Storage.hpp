#pragma once

#include <vector>

#include "EntityId.hpp"

namespace Sphynx::ECS {

	using ComponentIndex = uint32_t;
	constexpr ComponentIndex InvalidComponentIndex = (ComponentIndex)-1;

	// technically a move func
	using CopyFunc = void(*)(void* ptr, void* src);
	using DestroyFunc = void(*)(void* ptr);

	class Storage {
	public:
		Storage() = default;

		Storage(size_t elementSize, CopyFunc copyFunc, DestroyFunc destroyFunc)
			: m_ElementSize(elementSize), m_CopyFunc(copyFunc), m_DestroyFunc(destroyFunc) {}

		~Storage() {
			for (EntityId entity = 0; entity < (EntityId)m_ComponentIndexes.size(); entity++) {
				Remove(entity);
			}
		}

		template<typename T>
		T& Add(const EntityId entity, T&& srcComponent) {
			return *static_cast<T*>(AddRaw(entity, &srcComponent));
		}

		void* AddRaw(const EntityId entity, void* srcComponent) {
			if (Has(entity))
				return GetRaw(entity);

			if (entity >= m_ComponentIndexes.size())
				m_ComponentIndexes.resize((size_t)entity + 1, InvalidComponentIndex);

			ComponentIndex index = m_NextComponentIndex;
			m_NextComponentIndex += (ComponentIndex)m_ElementSize;
			if (m_Data.size() < m_NextComponentIndex)
				m_Data.resize(m_NextComponentIndex, 0);

			m_ComponentIndexes[entity] = index;
			void* componentPtr = &m_Data[index];
			m_CopyFunc(componentPtr, srcComponent);
			return componentPtr;
		}

		bool Has(const EntityId entity) const {
			if (entity >= m_ComponentIndexes.size())
				return false;
			return m_ComponentIndexes[entity] != InvalidComponentIndex;
		}

		void* TryGet(const EntityId entity) {
			if (entity >= m_ComponentIndexes.size())
				return nullptr;
			const ComponentIndex index = m_ComponentIndexes[entity];
			if (index == InvalidComponentIndex)
				return nullptr;
			return &m_Data[index];
		}

		template<typename T>
		T& Get(const EntityId entity) {
			return *static_cast<T*>(GetRaw(entity));
		}

		void* GetRaw(const EntityId entity) {
			if (entity >= m_ComponentIndexes.size())
				return nullptr;
			const ComponentIndex index = m_ComponentIndexes[entity];
			if (index == InvalidComponentIndex)
				return nullptr;

			return &m_Data[index];
		}

		bool Remove(const EntityId entity) {
			if (!Has(entity))
				return false;

			const ComponentIndex oldIndex = m_ComponentIndexes[entity];
			m_ComponentIndexes[entity] = InvalidComponentIndex;
			void* componentPtr = &m_Data[oldIndex];
			m_DestroyFunc(componentPtr);
			std::memset(componentPtr, 0, m_ElementSize);
			return true;
		}

		template<typename T>
		T& Replace(const EntityId entity, T&& srcComponent) {
			return *static_cast<T*>(ReplaceRaw(entity, &srcComponent));
		}

		void* ReplaceRaw(const EntityId entity, void* srcComponent) {
			if (!Has(entity))
				return AddRaw(entity, srcComponent);

			const ComponentIndex index = m_ComponentIndexes[entity];
			void* componentPtr = &m_Data[index];
			m_DestroyFunc(componentPtr);
			m_CopyFunc(componentPtr, &srcComponent);
			return componentPtr;
		}

		size_t Size() const {
			return m_Data.size() / m_ElementSize;
		}


		struct Iterator {
			Iterator(Storage& storage, EntityId entity) : m_Storage(storage), m_Entity(entity) {
				FindNext();
			}

			void* operator*() { return m_Current; }

			Iterator& operator++() {
				++m_Entity;
				FindNext();
				return *this;
			}

			bool operator==(const Iterator& other) const { return m_Entity == other.m_Entity; }

		private:
			void FindNext() {
				while (m_Entity < (EntityId)m_Storage.m_ComponentIndexes.size()) {
					const ComponentIndex index = m_Storage.m_ComponentIndexes[m_Entity];
					if (index != InvalidComponentIndex) {
						m_Current = &m_Storage.m_Data[index];
						break;
					}
					m_Entity++;
				}
			}

		private:
			Storage& m_Storage;
			EntityId m_Entity;
			void* m_Current = nullptr;
		};
		Iterator begin() { return Iterator{ *this, 0 }; }
		Iterator end() { return Iterator{ *this, (EntityId)m_ComponentIndexes.size() }; }

	private:
		size_t m_ElementSize = 0;
		CopyFunc m_CopyFunc = nullptr;
		DestroyFunc m_DestroyFunc = nullptr;

		ComponentIndex m_NextComponentIndex = 0;
		std::vector<uint8_t> m_Data; // Dense
		std::vector<ComponentIndex> m_ComponentIndexes; // Sparse
	};

}