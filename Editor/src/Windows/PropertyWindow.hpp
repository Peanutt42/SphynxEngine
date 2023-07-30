#pragma once

#include "EditorWindow.hpp"
#include "HierarchyWindow.hpp"

namespace Sphynx::Editor {
	class PropertyWindow : public EditorWindow {
	public:
		PropertyWindow() {
			Name = "Properties";
		}

		virtual void Draw() override {
			Scene& scene = EditorApplication::GetCurrentScene();
			ECS::EntityId entity = HierarchyWindow::s_SelectedEntity;
			if (!scene.IsValid(entity))
				return;

			ImGui::InputText("Name", &scene.GetComponent<ECS::NameComponent>(entity)->Name);

			if (BeginComponent<ECS::TransformComponent>("Transform", scene, entity)) {
				ECS::TransformComponent* transform = scene.GetComponent<ECS::TransformComponent>(entity);
				UI::Vec3("Position", transform->Position);
				UI::Vec3("Rotation", transform->Rotation);
				UI::Vec3("Scale", transform->Scale, 1.f);

				EndComponent();
			}
		}

	private:
		template<typename T>
		bool BeginComponent(std::string_view name, Scene& scene, ECS::EntityId entity) {
			if (!scene.HasComponent<T>(entity))
				return false;

			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.data());
			ImGui::PopStyleVar();
			
			
			//ImGui::SameLine(contentRegionAvailable.x - lineHeight / 1.25f);
			//ImGui::PushFont(Fonts::Get(Fonts::Type::ExtraBold));
			//float lineHeight = ImGui::GetFont()->FontSize;
			//if (UI::ImageButton(EditorApplication::IconLibary().GetIconTexture(Icon::OptionsIcon), ImVec2{ lineHeight, lineHeight }))
			//	ImGui::OpenPopup("ComponentSettings");

			//ImGui::PopFont();

			if (ImGui::BeginPopup("ComponentSettings")) {
				if (ImGui::MenuItem("Remove component")) {
					EditorApplication::SetSceneDirty(true);
					scene.RemoveComponent<T>(entity);
				}

				ImGui::EndPopup();
			}

			return open;
		}

		void EndComponent() {
			ImGui::TreePop();
		}
	};
}