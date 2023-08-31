#pragma once

#include <cstdint>
#include <string_view>

namespace Sphynx::ECS {
	namespace internal {
		template<typename Type>
		constexpr std::string_view function_name() {
#ifdef __clang__
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#error "Compiler not supported!"
#endif
		}

		constexpr auto void_prober = function_name<void>();
		constexpr auto start = void_prober.find("void");
		constexpr auto offset = void_prober.length() - 4;

		template<typename Type>
		constexpr std::string_view type_name() {
			constexpr auto raw = function_name<Type>();

			return raw.substr(start, raw.length() - offset);
		}

		static constexpr uint64_t basis = 14695981039346656037ull;
		static constexpr uint64_t prime = 1099511628211ull;

		constexpr uint64_t hash(uint64_t n, const char* string, uint64_t h = basis) {
			return n > 0 ? hash(n - 1, string + 1, (h ^ *string) * prime) : h;
		}

		constexpr uint32_t hash(const std::string_view view) {
			return static_cast<uint32_t>(hash(view.length() - 1, view.data()));
		}

	}


	using ComponentId = uint32_t;
	constexpr ComponentId InvalidComponentId = static_cast<ComponentId>(-1);

	template<typename T>
	constexpr ComponentId GetComponentId() {
		return internal::hash(internal::type_name<T>());
	}
}