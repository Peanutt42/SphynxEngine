#pragma once

#include "std.hpp"

namespace Sphynx {
	struct Version {
		int Major = 0;
		int Minor = 0;
		int Patch = 0;

		constexpr Version() = default;
		constexpr Version(int major, int minor, int patch) : Major(major), Minor(minor), Patch(patch) {}

		bool operator==(const Version&) const = default;

		std::string to_string() const {
			return std::to_string(Major) + ", " + std::to_string(Minor) + ", " + std::to_string(Patch);
		}
	};
}