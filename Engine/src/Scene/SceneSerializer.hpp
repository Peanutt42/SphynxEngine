#pragma once

#include "Core/CoreInclude.hpp"
#include "Scene/DefaultComponents.hpp"
#include "Serialization/YAMLSerializer.hpp"
#include "Scene.hpp"

namespace Sphynx {
	class SceneSerializer {
	public:
		static void SerializeEntity(Scene& scene, ECS::EntityId entity, YAML::Emitter& out) {
			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << scene.GetComponent<ECS::UUIDComponent>(entity)->uuid;
			out << YAML::Key << "Name" << YAML::Value << scene.GetComponent<ECS::NameComponent>(entity)->Name;

			out << YAML::EndMap;
		}

		static void Serialize(const std::filesystem::path& filepath, Scene& scene) {
			YAML::Emitter out;
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << scene.GetName();

			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			auto entityView = scene.View<ECS::UUIDComponent>();
			entityView.ForEach([&](ECS::EntityId entity) {
				SerializeEntity(scene, entity, out);
			});
			out << YAML::EndSeq;

			out << YAML::EndMap;

			YAMLSerializer::SaveFile(filepath, out);
		}


		static void DeserializeEntity(Scene& scene, const YAML::Node& entityNode) {
			UUID uuid = entityNode["Entity"].as<UUID>();
			ECS::EntityId entity = scene.CreateEntity(uuid);
			scene.GetComponent<ECS::NameComponent>(entity)->Name = entityNode["Name"].as<std::string>();
		}

		static void Deserialize(const std::filesystem::path& filepath, Scene& outScene) {
			std::string filepathStr = filepath.string();
			
			YAML::Node data;
			SE_ASSERT(YAMLSerializer::LoadFile(filepathStr.c_str(), data), Logging::Serialization, "Failed to open scene file {}", filepathStr);

			outScene.SetName(data["Name"].as<std::string>());

			auto entitiesNode = data["Entities"];
			for (const YAML::Node& entityNode : entitiesNode) {
				DeserializeEntity(outScene, entityNode);
			}
		}
	};
}