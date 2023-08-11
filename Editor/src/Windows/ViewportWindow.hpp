#pragma once

#include "EditorWindow.hpp"
#include "Rendering/Renderer.hpp"

namespace Sphynx::Editor {
	class ViewportWindow : public EditorWindow {
	public:
		ViewportWindow() {
			Name = "Viewport";
		}

		virtual void Draw() override {
			ImVec2 rect = ImGui::GetContentRegionAvail();
			ImGui::Image((ImTextureID)Engine::Renderer().GetSceneTextureID(), rect);
		}
	};
}