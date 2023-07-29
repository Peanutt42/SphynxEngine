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
		}
	};
}