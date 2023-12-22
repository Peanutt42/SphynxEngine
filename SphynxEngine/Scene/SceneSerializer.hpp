#pragma once

#include "pch.hpp"
#include "Serialization/YAMLSerializer.hpp"
#include "Scene.hpp"
#include "Scene/AllComponents.hpp"

namespace Sphynx {
	class SceneSerializer {
	public:
		static void SerializeEntity(Scene& scene, entt::entity entity, YAML::Emitter& out) {
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

			if (auto* light = scene.GetComponent<Rendering::LightComponent>(entity)) {
				out << YAML::Key << "Light" << YAML::BeginMap;
				out << YAML::Key << "Color" << YAML::Value << light->Color;
				out << YAML::EndMap;
			}
			if (auto* camera = scene.GetComponent<Rendering::CameraComponent>(entity)) {
				out << YAML::Key << "Camera" << YAML::BeginMap;
				out << YAML::Key << "FOV" << YAML::Value << camera->FOV;
				out << YAML::Key << "NearPlane" << YAML::Value << camera->NearPlane;
				out << YAML::Key << "FarPlane" << YAML::Value << camera->FarPlane;
				out << YAML::EndMap;
			}
			if (auto* mesh = scene.GetComponent<Rendering::MeshComponent>(entity)) {
				out << YAML::Key << "Mesh" << YAML::BeginMap;
				out << YAML::EndMap;
			}

			out << YAML::EndMap;
		}

		static void Serialize(const std::filesystem::path& filepath, Scene& scene) {
			YAML::Emitter out;
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << scene.GetName();

			out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
			scene.ForEach([&](entt::entity entity) {
				SerializeEntity(scene, entity, out);
			});
			out << YAML::EndSeq;

			out << YAML::EndMap;

			YAMLSerializer::SaveFile(filepath, out);
		}

#
		static void DeserializeEntity(Scene& scene, const YAML::Node& entityNode) {
			UUID uuid = entityNode["Entity"].as<UUID>();
			entt::entity entity = scene.CreateEntity(uuid);
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

			if (YAML::Node lightNode = entityNode["Light"]) {
				Rendering::LightComponent light;
				light.Color = lightNode["Color"].as<glm::vec3>();

				scene.AddComponent<Rendering::LightComponent>(entity, light);
			}
			if (YAML::Node cameraNode = entityNode["Camera"]) {
				Rendering::CameraComponent camera;
				camera.FOV = cameraNode["FOV"].as<float>();
				camera.NearPlane = cameraNode["NearPlane"].as<float>();
				camera.FarPlane = cameraNode["FarPlane"].as<float>();

				scene.AddComponent<Rendering::CameraComponent>(entity, camera);
			}
			if (YAML::Node meshNode = entityNode["Mesh"]) {
				Rendering::MeshComponent mesh;
				scene.AddComponent(entity, mesh);
			}
		}

		static Result<bool, std::string> Deserialize(const std::filesystem::path& filepath, Scene& outScene) {
			std::string filepathStr = filepath.string();
			
			auto result = YAMLSerializer::LoadFile(filepathStr.c_str());
			if (result.is_error())
				return Error<bool>("Failed to open scene file {}: {}", filepathStr, result.get_error());

			try {
				YAML::Node& data = *result;

				outScene.SetName(data["Name"].as<std::string>());

				auto entitiesNode = data["Entities"];
				for (const YAML::Node& entityNode : entitiesNode) {
					DeserializeEntity(outScene, entityNode);
				}
			}
			catch (const std::exception& e) {
				return Error<bool>("Failed to parse scene file {} ({})", filepathStr, e.what());
			}

			return true;
		}
	};
}