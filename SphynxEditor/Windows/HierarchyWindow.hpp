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
			scene.ForEach([&](entt::entity entity) {
				ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
				if (s_SelectedEntity == entity)
					flags |= ImGuiTreeNodeFlags_Selected;

				const std::string& name = scene.GetComponent<ECS::NameComponent>(entity)->Name;
				bool opened = ImGui::TreeNodeEx((void*)(uint64)(uint32)entity, flags, "%s", name.c_str());

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
					s_SelectedEntity = entt::null;
				}
			});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				s_SelectedEntity = entt::null;

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow("EntityCreateMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_NoOpenOverExistingPopup)) {
				if (ImGui::MenuItem("Create Entity")) {
					entt::entity  entity = EditorApplication::GetCurrentScene().CreateEntity();
					s_SelectedEntity = entity;
					EditorApplication::SetSceneDirty(true);
				}
				if (s_CopiedEntity != entt::null && ImGui::MenuItem("Paste Entity")) {
					s_SelectedEntity = EditorApplication::GetCurrentScene().DublicateEntity(s_CopiedEntity);
					EditorApplication::SetSceneDirty(true);
				}

				ImGui::EndPopup();
			}
		}

		inline static entt::entity s_SelectedEntity = entt::null;
		inline static entt::entity s_CopiedEntity = entt::null;
	};
}