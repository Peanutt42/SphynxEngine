#pragma once

#include "pch.hpp"
#include "UI/UI.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

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