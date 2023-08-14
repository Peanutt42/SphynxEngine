#pragma once

#include "EditorWindow.hpp"
#include "EditorApplication.hpp"
#include "Scene/DefaultComponents.hpp"

namespace Sphynx::Editor {
	class HierarchyWindow : public EditorWindow {
	public:
		HierarchyWindow() {
			Name = "Hierarchy";
		}

		virtual void Draw() override {
			Scene& scene = EditorApplication::GetCurrentScene();
			scene.ForEach([&](ECS::EntityId entity) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (s_SelectedEntity == entity)
					flags |= ImGuiTreeNodeFlags_Selected;

				const std::string& name = scene.GetComponent<ECS::NameComponent>(entity)->Name;
				bool opened = ImGui::TreeNodeEx((void*)(uint32)entity, flags, name.c_str());

				if (ImGui::IsItemClicked())
					s_SelectedEntity = entity;

				// TODO: Drag Drop Entities

				bool deleteEntity = false;
				if (ImGui::BeginPopupContextItem()) {
					deleteEntity = ImGui::MenuItem("Delete Entity");
					if (deleteEntity) EditorApplication::SetSceneDirty(true);

					if (ImGui::MenuItem("Copy Entity"))
						s_CopiedEntity = entity;

					ImGui::EndPopup();
				}
				if (opened)
					ImGui::TreePop();

				if (deleteEntity) {
					scene.DestroyEntity(entity);
					s_SelectedEntity = ECS::InvalidEntityId;
				}
			});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				s_SelectedEntity = ECS::InvalidEntityId;

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow("EntityCreateMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
				if (ImGui::MenuItem("Create Entity")) {
					ECS::EntityId entity = EditorApplication::GetCurrentScene().CreateEntity();
					s_SelectedEntity = entity;
					EditorApplication::SetSceneDirty(true);
				}
				if (ImGui::MenuItem("Paste Entity", "", nullptr, s_CopiedEntity)) {
					s_SelectedEntity = EditorApplication::GetCurrentScene().DublicateEntity(s_CopiedEntity);
					EditorApplication::SetSceneDirty(true);
				}

				ImGui::EndPopup();
			}
		}

		inline static ECS::EntityId s_SelectedEntity = ECS::InvalidEntityId;
		inline static ECS::EntityId s_CopiedEntity = ECS::InvalidEntityId;
	};
}