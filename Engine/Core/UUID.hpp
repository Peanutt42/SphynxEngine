#pragma once

#include "std.hpp"
#include "IntTypes.hpp"

namespace Sphynx {
	struct UUID {
		using type = uint64;

		UUID() : m_UUID(s_UniformDistribution(s_Engine)) {}
		UUID(type uuid) : m_UUID(uuid) {}
		UUID(const UUID&) = default;
		UUID(UUID&&) = default;

		UUID& operator=(const UUID&) = default;
		UUID& operator=(UUID&&) = default;
		UUID& operator=(type uuid) { m_UUID = uuid; return *this; }

		operator type() const { return m_UUID; }

	private:
		type m_UUID = 0;

		inline static std::random_device s_RandomDevice;
		inline static std::mt19937_64 s_Engine = std::mt19937_64(s_RandomDevice());
		inline static std::uniform_int_distribution<type> s_UniformDistribution;
	};
}