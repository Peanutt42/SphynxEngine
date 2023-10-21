#pragma once

#include "std.hpp"

namespace Sphynx {
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using byte = uint8_t;
	static_assert(sizeof(byte) == 1);

	using intptr = intptr_t;
	using uintptr = uintptr_t;


	template<typename T>
	std::optional<T> StringToNumber(std::string_view str) {
		T result{};
		auto[ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
		if (ec == std::errc())
			return result;
		else
			return std::nullopt;
	}
}