#pragma once

#include <vector>

#include "EntityId.hpp"

namespace Sphynx::ECS {

	using ComponentIndex = uint32_t;
	constexpr ComponentIndex InvalidComponentIndex = (ComponentIndex)-1;

	class Storage {
	public:
		Storage() = default;

		Storage(size_t elementSize) : m_ElementSize(elementSize) {}

		template<typename T>
		T& Add(const EntityId entity) {
			return *static_cast<T*>(Add(entity));
		}

		void* Add(const EntityId entity) {
			if (Has(entity))
				return Get(entity);

			if (entity >= m_ComponentIndexes.size())
				m_ComponentIndexes.resize(entity + 1, InvalidComponentIndex);

			ComponentIndex index = m_NextComponentIndex;
			m_NextComponentIndex += (ComponentIndex)m_ElementSize;
			if (m_Data.size() < m_NextComponentIndex)
				m_Data.resize(m_NextComponentIndex, 0);

			m_ComponentIndexes[entity] = index;
			return &m_Data[index];
		}

		bool Has(const EntityId entity) const {
			if (entity >= m_ComponentIndexes.size())
				return false;
			return m_ComponentIndexes[entity] != InvalidComponentIndex;
		}

		bool TryGet(const EntityId entity, void** out) {
			if (entity >= m_ComponentIndexes.size())
				return false;
			const ComponentIndex index = m_ComponentIndexes[entity];
			if (index == InvalidComponentIndex)
				return false;
			*out = &m_Data[index];
			return true;
		}

		template<typename T>
		T& Get(const EntityId entity) {
			return *static_cast<T*>(Get(entity));
		}

		void* Get(const EntityId entity) {
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
			std::memset(&m_Data[oldIndex], 0, m_ElementSize);
			return true;
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
		ComponentIndex m_NextComponentIndex = 0;
		std::vector<uint8_t> m_Data; // Dense
		std::vector<ComponentIndex> m_ComponentIndexes; // Sparse
	};

}