#pragma once

#if defined(DEBUG) || defined(DEVELOPMENT)

#ifndef TRACY_ENABLE
#define TRACY_ENABLE true
#endif
#ifndef TRACY_ON_DEMAND
#define TRACY_ON_DEMAND
#endif
#include <tracy/Tracy.hpp>

#define SE_PROFILE_FRAME_END(name) FrameMarkNamed(name)

namespace Sphynx::Profiling {
	constexpr std::string_view GetCleanFunction(std::string_view str) {
		size_t end = str.find('(');
		size_t start = str.rfind(' ', end);
		return str.substr(start + 1, end - start - 1);
	}

	template<size_t N1, size_t N2>
	constexpr auto CompileTimeStringConcat(std::string_view s1, const char (&s2)[N2]) {
		std::array<char, N1 + N2 - 1> result{};

		for (std::size_t i = 0; i < N1 - 1; ++i)
			result[i] = s1[i];
		for (std::size_t i = 0; i < N2; ++i)
			result[N1 - 1 + i] = s2[i];

		return result;
	}

	template<size_t N1, size_t N2>
	constexpr auto CompileTimeStringConcat(const char (&s1)[N1], const char (&s2)[N2]) {
		std::array<char, N1 + N2 - 1> result{};

		for (std::size_t i = 0; i < N1 - 1; ++i)
			result[i] = s1[i];
		for (std::size_t i = 0; i < N2; ++i)
			result[N1 - 1 + i] = s2[i];

		return result;
	}

#if !defined(_MSC_VER)
	class Foo { static void Bar(int& a) { static_assert(GetCleanFunction(__PRETTY_FUNCTION__) == "Sphynx::Profiling::Foo::Bar"); }};
#endif
}

#if !defined(_MSC_VER)
#define SE_PROFILE_FUNCTION() \
	constexpr static auto TracyConcat(____function_name, TracyLine) = Sphynx::Profiling::GetCleanFunction(__PRETTY_FUNCTION__); \
	ZoneScopedN(TracyConcat(____function_name, TracyLine).data())

// Adds the name to the current function name
#define SE_PROFILE_SCOPE(name) \
	constexpr static auto TracyConcat(____function_name, TracyLine) = Sphynx::Profiling::GetCleanFunction(__PRETTY_FUNCTION__); \
	constexpr static auto TracyConcat(____scope_name, TracyLine) = Sphynx::Profiling::CompileTimeStringConcat<TracyConcat(____function_name, TracyLine).size() + 1>(TracyConcat(____function_name, TracyLine), "::" name); \
	ZoneNamedN(____tracy_scoped_zone, TracyConcat(____scope_name, TracyLine).data(), true)
#else
#define SE_PROFILE_FUNCTION() ZoneScoped;
// Adds the name to the current function name
#define SE_PROFILE_SCOPE(name) \
	constexpr static auto TracyConcat(____scope_name, TracyLine) = Sphynx::Profiling::CompileTimeStringConcat(TracyFunction, "::" name); \
	ZoneNamedN(____tracy_scoped_zone, TracyConcat(____scope_name, TracyLine).data(), true)
#endif

#else

#define SE_PROFILE_FRAME_END(name)

#define SE_PROFILE_FUNCTION()
#define SE_PROFILE_SCOPE(name)

#endif