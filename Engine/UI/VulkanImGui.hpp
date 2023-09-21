#pragma once

#include "pch.hpp"
#include "Rendering/Image.hpp"
#include <imgui.h>

namespace Sphynx::UI {
	class SE_API VulkanImGui {
	public:
		static void Init();
		static void Shutdown();

		static void Begin();
		static void End();

		static void EnableDocking();

		static void Render();

		static void SetSaveFilepath(const std::filesystem::path& filepath);
		static void DisableSaveFile();

		static void SetMenubarCallback(const std::function<void()>& callback);

		static ImGuiContext* GetContext();
		
		static void _DrawTitlebar();
	};
}