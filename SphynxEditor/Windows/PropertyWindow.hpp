#pragma once

#include "EditorWindow.hpp"
#include "HierarchyWindow.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "Scene/AllComponents.hpp"

namespace Sphynx::Editor {
	class PropertyWindow : public EditorWindow {
	public:
		PropertyWindow() {
			Name = "Properties";
		}

		virtual void Draw() override {
			SE_PROFILE_FUNCTION();

			Scene& scene = EditorApplication::GetCurrentScene();
			entt::entity entity = HierarchyWindow::s_SelectedEntity;
			if (!scene.IsValid(entity))
				return;

			ImGui::InputText("Name", &scene.GetComponent<ECS::NameComponent>(entity)->Name);

			if (BeginComponent<ECS::TransformComponent>("Transform", scene, entity)) {
				ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity);
				UI::Vec3("Position", transform->Position);
				glm::vec3 rotation = glm::degrees(transform->Rotation);
				if (UI::Vec3("Rotation", rotation))
					transform->Rotation = glm::radians(rotation);
				UI::Vec3("Scale", transform->Scale, 1.f);

				EndComponent();
			}

			if (BeginComponent<Physics::RigidbodyComponent>("Rigidbody", scene, entity)) {
				Physics::RigidbodyComponent* rb = scene.GetComponent<Physics::RigidbodyComponent>(entity);
				ImGui::Checkbox("Dynamic", &rb->Dynamic);

				EndComponent();
			}
			if (BeginComponent<Physics::BoxCollider>("BoxCollider", scene, entity)) {
				Physics::BoxCollider* box = scene.GetComponent<Physics::BoxCollider>(entity);
				UI::Vec3("HalfExtent", box->HalfExtent, 1.f);

				EndComponent();
			}
			if (BeginComponent<Physics::SphereCollider>("SphereCollider", scene, entity)) {
				Physics::SphereCollider* sphere = scene.GetComponent<Physics::SphereCollider>(entity);
				ImGui::DragFloat("Radius", &sphere->Radius);

				EndComponent();
			}

			if (BeginComponent<Rendering::LightComponent>("Light", scene, entity)) {
				auto* light = scene.GetComponent<Rendering::LightComponent>(entity);
				ImGui::ColorEdit3("Color", glm::value_ptr(light->Color));

				EndComponent();
			}
			if (BeginComponent<Rendering::CameraComponent>("Camera", scene, entity)) {
				auto* camera = scene.GetComponent<Rendering::CameraComponent>(entity);
				ImGui::SliderFloat("FOV", &camera->FOV, 0.f, 180.f);
				ImGui::DragFloat("NearPlane", &camera->NearPlane);
				ImGui::DragFloat("FarPlane", &camera->FarPlane);

				EndComponent();
			}
			if (BeginComponent<Rendering::MeshComponent>("Mesh", scene, entity)) {
				auto* mesh = scene.GetComponent<Rendering::MeshComponent>(entity);
				ImGui::ColorEdit3("Albedo", glm::value_ptr(mesh->albedo));
				ImGui::SliderFloat("Metalic", &mesh->metalic, 0.f, 1.f);
				ImGui::SliderFloat("Roughness", &mesh->roughness, 0.f, 1.f);

				EndComponent();
			}


			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent")) {
				if (ImGui::BeginMenu("Physics")) {
					DisplayAddComponentEntry<Physics::RigidbodyComponent>("Rigidbody");
					DisplayAddComponentEntry<Physics::BoxCollider>("BoxCollider");
					DisplayAddComponentEntry<Physics::SphereCollider>("SphereCollider");

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Rendering")) {
					DisplayAddComponentEntry<Rendering::LightComponent>("Light");
					DisplayAddComponentEntry<Rendering::CameraComponent>("Camera");
					DisplayAddComponentEntry<Rendering::MeshComponent>("Mesh");

					ImGui::EndMenu();
				}

				ImGui::EndPopup();
			}
		}

	private:
		template<typename T>
		bool BeginComponent(std::string_view name, Scene& scene, entt::entity entity) {
			if (!scene.HasComponent<T>(entity))
				return false;

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen;

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.data());
			ImGui::PopStyleVar();
			
			
			float lineHeight = ImGui::GetFont()->FontSize;
			ImGui::SameLine(contentRegionAvailable.x - lineHeight / 1.25f);
			//ImGui::PushFont(Fonts::Get(Fonts::Type::ExtraBold));
			if (ImGui::Button("Settings", ImVec2{ lineHeight, lineHeight }))//if (UI::ImageButton(EditorApplication::IconLibary().GetIconTexture(Icon::OptionsIcon), ImVec2{ lineHeight, lineHeight }))
				ImGui::OpenPopup("ComponentSettings");
			//ImGui::PopFont();


			if (ImGui::BeginPopup("ComponentSettings")) {
				bool removedComponent = false;
				if (ImGui::MenuItem("Remove component")) {
					EditorApplication::SetSceneDirty(true);
					scene.RemoveComponent<T>(entity);
					removedComponent = true;
				}

				ImGui::EndPopup();
				if (removedComponent) {
					EndComponent();
					return false;
				}
			}

			return open;
		}

		void EndComponent() {
			ImGui::TreePop();
		}

		template<typename T, typename... Args>
		bool DisplayAddComponentEntry(const std::string_view entryName, Args&&... args) {
			Scene& scene = EditorApplication::GetCurrentScene();
			if (!scene.HasComponent<T>(HierarchyWindow::s_SelectedEntity)) {
				if (ImGui::MenuItem(entryName.data())) {
					scene.AddComponent<T>(HierarchyWindow::s_SelectedEntity, T(std::forward<Args>(args)...));
					EditorApplication::SetSceneDirty(true);
					ImGui::CloseCurrentPopup();
					return true;
				}
			}
			return false;
		}
	};
}