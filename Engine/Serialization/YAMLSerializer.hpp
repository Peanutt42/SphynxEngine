#pragma once

#include "pch.hpp"
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
	struct convert<glm::vec2> {
		static Node encode(const glm::vec2& v) {
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, glm::vec2& v) {
			if (!node.IsSequence() || node.size() != 2)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();

			return true;
		}
	};
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	template<>
	struct convert<glm::vec3> {
		static Node encode(const glm::vec3& v) {
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, glm::vec3& v) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();

			return true;
		}
	};
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	template<>
	struct convert<glm::vec4> {
		static Node encode(const glm::vec4& v) {
			Node node;
			node.push_back(v.x);
			node.push_back(v.y);
			node.push_back(v.z);
			node.push_back(v.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, glm::vec4& v) {
			if (!node.IsSequence() || node.size() != 4)
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();
			v.w = node[3].as<float>();

			return true;
		}
	};
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	template<>
	struct convert<Sphynx::UUID> {
		static Node encode(const Sphynx::UUID& uuid) {
			Node node;
			node.push_back((Sphynx::UUID::type)uuid);
			return node;
		}
		static bool decode(const Node& node, Sphynx::UUID& uuid) {
			uuid = node.as<Sphynx::UUID::type>();
			return true;
		}
	};
}

namespace Sphynx {
	class SE_API YAMLSerializer {
	public:
		static bool SaveFile(const std::filesystem::path& filepath, const YAML::Emitter& emitter) {
			std::ofstream fout(filepath);
			if (!fout) {
				SE_ERR("Failed to save file '{}'", filepath.string());
				return false;
			}

			fout << emitter.c_str();
			fout.close();
			return true;
		}

		static Result<YAML::Node, std::string> LoadFile(const std::filesystem::path& filepath) {
			if (!std::filesystem::exists(filepath))
				return Error<YAML::Node>("{} doesn't exist!", filepath.string());

			YAML::Node data;
			try {
				data = YAML::LoadFile(filepath.string());
			}
			catch (const YAML::Exception& e) {
				return Error<YAML::Node>("Failed to open yaml file {},\n    yaml exception: {}", filepath.string(), e.what());
			}
			return data;
		}
	};
}