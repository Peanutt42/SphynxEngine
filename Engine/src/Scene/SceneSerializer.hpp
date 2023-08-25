#pragma once

#include "pch.hpp"
#include "Serialization/YAMLSerializer.hpp"
#include "Scene.hpp"
#include "Scene/DefaultComponents.hpp"
#include "Physics/PhysicsComponents.hpp"

namespace Sphynx {
	class SceneSerializer {
	public:
		static void SerializeEntity(Scene& scene, ECS::EntityId entity, YAML::Emitter& out) {
			out << YAML::BeginMap;
			out << YAML::Key << "Entity" << YAML::Value << scene.GetComponent<ECS::UUIDComponent>(entity)->uuid;
			out << YAML::Key << "Name" << YAML::Value << scene.GetComponent<ECS::NameComponent>(entity)->Name;

			if (ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity)) {
				out << YAML::Key << "Transform" << YAML::BeginMap;
				out << YAML::Key << "Position" << YAML::Value << transform->Position;
				out << YAML::Key << "Rotation" << YAML::Value << transform->Rotation;
				out << YAML::Key << "Scale" << YAML::Value << transform->Scale;

				out << YAML::EndMap;
			}

			if (Physics::RigidbodyComponent* rb = scene.GetComponent<Physics::RigidbodyComponent>(entity)) {
				out << YAML::Key << "Rigidbody" << YAML::BeginMap;
				out << YAML::Key << "Dynamic" << YAML::Value << rb->Dynamic;
				out << YAML::EndMap;
			}
			if (Physics::BoxCollider* box = scene.GetComponent<Physics::BoxCollider>(entity)) {
				out << YAML::Key << "BoxCollider" << YAML::BeginMap;
				out << YAML::Key << "HalfExtent" << YAML::Value << box->HalfExtent;
				out << YAML::EndMap;
			}
			if (Physics::SphereCollider* sphere = scene.GetComponent<Physics::SphereCollider>(entity)) {
				out << YAML::Key << "SphereCollider" << YAML::BeginMap;
				out << YAML::Key << "Radius" << YAML::Value << sphere->Radius;
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
			
			if (YAML::Node transformNode = entityNode["Transform"]) {
				ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity);
				transform->Position = transformNode["Position"].as<glm::vec3>();
				transform->Rotation = transformNode["Rotation"].as<glm::vec3>();
				transform->Scale = transformNode["Scale"].as<glm::vec3>();
			}

			if (YAML::Node rigidbodyNode = entityNode["Rigidbody"]) {
				Physics::RigidbodyComponent rb;
				rb.Dynamic = rigidbodyNode["Dynamic"].as<bool>();

				scene.AddComponent<Physics::RigidbodyComponent>(entity, rb);
			}
			if (YAML::Node boxNode = entityNode["BoxCollider"]) {
				Physics::BoxCollider box;
				box.HalfExtent = boxNode["HalfExtent"].as<glm::vec3>();

				scene.AddComponent<Physics::BoxCollider>(entity, box);
			}
			if (YAML::Node sphereNode = entityNode["SphereCollider"]) {
				Physics::SphereCollider sphere;
				sphere.Radius = sphereNode["Radius"].as<float>();

				scene.AddComponent<Physics::SphereCollider>(entity, sphere);
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