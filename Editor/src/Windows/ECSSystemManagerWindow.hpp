#pragma once

#include "EditorWindow.hpp"
#include "Core/Engine.hpp"
#include "Scripting/ScriptingEngine.hpp"

namespace Sphynx::Editor {
	class ECSSystemManagerWindow : public EditorWindow {
	public:
		ECSSystemManagerWindow() {
			Name = "ECS SystemManager";
		}

		virtual void Draw() override {
			for (const auto& system : Engine::Scripting().GetSystems()) {
				bool active = EditorApplication::GetECSSystemActive(system.FullName);
				if (ImGui::Checkbox("##", &active))
					EditorApplication::SetECSSystemActive(system.FullName, active);
				ImGui::SameLine();
				ImGui::Text("%s", system.FullName.c_str());
			}
		}
	};
}