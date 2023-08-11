#pragma once

#include "pch.hpp"
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

			if (scene.HasComponent<ECS::TransformComponent>(entity)) {
				out << YAML::Key << "Transform" << YAML::BeginMap;

				ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity);
				out << YAML::Key << "Position" << YAML::Value << transform->Position;
				out << YAML::Key << "Rotation" << YAML::Value << transform->Rotation;
				out << YAML::Key << "Scale" << YAML::Value << transform->Scale;

				out << YAML::EndMap;
			}

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

#
		static void DeserializeEntity(Scene& scene, const YAML::Node& entityNode) {
			UUID uuid = entityNode["Entity"].as<UUID>();
			ECS::EntityId entity = scene.CreateEntity(uuid);
			scene.GetComponent<ECS::NameComponent>(entity)->Name = entityNode["Name"].as<std::string>();
			
			YAML::Node transformNode = entityNode["Transform"];
			if (transformNode) {
				ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity);
				transform->Position = transformNode["Position"].as<glm::vec3>();
				transform->Rotation = transformNode["Rotation"].as<glm::vec3>();
				transform->Scale = transformNode["Scale"].as<glm::vec3>();
			}
		}

		static bool Deserialize(const std::filesystem::path& filepath, Scene& outScene, std::string& outErrorMsg) {
			std::string filepathStr = filepath.string();
			
			YAML::Node data;
			if (!YAMLSerializer::LoadFile(filepathStr.c_str(), data)) {
				outErrorMsg = std::format("Failed to open scene file {}", filepathStr);
				return false;
			}

			try {
				outScene.SetName(data["Name"].as<std::string>());

				auto entitiesNode = data["Entities"];
				for (const YAML::Node& entityNode : entitiesNode) {
					DeserializeEntity(outScene, entityNode);
				}
			}
			catch (const std::exception& e) {
				outErrorMsg = e.what();
				return false;
			}
			return true;
		}
	};
}