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
			ImGui::Text("Total: %.3f ms", EditorApplication::GetECSSystemDeltaTime() * 1000.f);

			for (const auto& system : Scripting::ScriptingEngine::GetSystems()) {
				const ECSGameSystemInfo* systemInfo = EditorApplication::GetECSSystem(system.FullName);
				if (!systemInfo)
					continue;

				bool active = systemInfo->Active;
				if (ImGui::Checkbox("##", &active))
					EditorApplication::SetECSSystemActive(system.FullName, active);
				ImGui::SameLine();
				ImGui::Text("%s: %.3f ms", system.FullName.c_str(), systemInfo->LastDeltatime * 1000.f);
			}
		}
	};
}