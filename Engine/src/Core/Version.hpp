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
	};
}

namespace std {
	template<class CharT>
	struct std::formatter<Sphynx::Version, CharT> : std::formatter<std::basic_string<CharT>, CharT> {
		template <typename FormatContext>
		auto format(const Sphynx::Version& version, FormatContext& ctx) {
			return std::formatter<std::basic_string<CharT>, CharT>::format(
				std::format("{}.{}.{}", version.Major, version.Minor, version.Patch), ctx);
		}
	};
}