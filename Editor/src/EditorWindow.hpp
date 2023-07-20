#pragma once

#include "Core/CoreInclude.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

namespace Sphynx::Editor {
	class EditorWindow {
	public:
		virtual ~EditorWindow() {}

		virtual void Update() {}
		virtual void Draw() = 0;

		std::string Name = "No Name";
		bool Opened = true;
	};
}