#pragma once

#include "pch.hpp"
#include <imgui.h>

namespace Sphynx::UI {
	class Fonts {
	public:
		enum class Type {
			Regular = 0,
			Bold = 1,
			ExtraBold = 2
		};

		static void Init() {
			ImGuiIO& io = ImGui::GetIO();
			constexpr float defaultSize = 18.f;

			auto regular = io.Fonts->AddFontFromFileTTF("Content/Fonts/OpenSans/static/OpenSans-Regular.ttf", defaultSize);
			io.Fonts->AddFontFromFileTTF("Content/Fonts/OpenSans/static/OpenSans-Bold.ttf", defaultSize);
			io.Fonts->AddFontFromFileTTF("Content/Fonts/OpenSans/static/OpenSans-ExtraBold.ttf", defaultSize);

			io.FontDefault = regular;
		}

		static ImFont* Get(Type type) {
			return ImGui::GetIO().Fonts->Fonts[(int)type];
		}
	};
}