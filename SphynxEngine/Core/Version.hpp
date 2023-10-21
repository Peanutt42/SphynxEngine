#pragma once

#include "std.hpp"
#include <fmt/core.h>

namespace Sphynx {
	struct Version {
		constexpr Version() = default;
		constexpr Version(int major, int minor, int patch) : Major(major), Minor(minor), Patch(patch) {}

		constexpr bool operator==(const Version&) const = default;
		constexpr bool operator>(const Version& other) const {
			if (Major > other.Major)
				return true;
			if (Minor > other.Minor)
				return true;
			return Patch > other.Patch;
		}
		constexpr bool operator<(const Version& other) const {
			if (Major < other.Major)
				return true;
			if (Minor < other.Minor)
				return true;
			return Patch < other.Patch;
		}

		std::string ToString() const {
			return fmt::format("{}.{}.{}", Major, Minor, Patch);
		}

		int Major = 0;
		int Minor = 0;
		int Patch = 0;
	};

	// Tests
	static_assert(Version(1,0,1) > Version(1,0,0));
	static_assert(Version(0,0,1) < Version(1,0,0));
	static_assert(Version(0,0,1) != Version(1,0,0));
}