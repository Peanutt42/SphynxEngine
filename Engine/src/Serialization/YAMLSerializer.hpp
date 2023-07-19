#pragma once

#include "Core/CoreInclude.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
	template<>
	struct convert<Sphynx::Version>	{
		static Node encode(const Sphynx::Version& version) {
			Node node;
			node.push_back(version.Major);
			node.push_back(version.Minor);
			node.push_back(version.Patch);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, Sphynx::Version& version) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			version.Major = node[0].as<int>();
			version.Minor = node[1].as<int>();
			version.Patch = node[2].as<int>();

			return true;
		}
	};

	template<>
	struct convert<Sphynx::UUID> {
		static Node encode(const Sphynx::UUID& uuid) {
			Node node;
			node.push_back((Sphynx::UUID::type)uuid);
			return node;
		}

		static bool decode(const Node& node, Sphynx::UUID& uuid) {
			if (node.size() != 1)
				return false;

			uuid = node.as<Sphynx::UUID::type>();
			return true;
		}
	};
}

namespace Sphynx {
	class SE_API Serialization {
	public:
		static bool SaveYamlToFile(const std::filesystem::path& filepath, const YAML::Emitter& emitter) {
			std::ofstream fout(filepath);
			if (!fout) {
				SE_ERR("Failed to save file '{}'", filepath.string());
				return false;
			}

			fout << emitter.c_str();
			fout.close();
			return true;
		}
	};
}