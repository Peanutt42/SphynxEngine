#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Image.hpp"
#include <imgui.h>

namespace Sphynx::UI {
	class SE_API VulkanImGuiHelper {
	public:
		VulkanImGuiHelper();
		~VulkanImGuiHelper();

		void Begin();
		void End();

		void EnableDocking();

		void Render();

		void SetSaveFilepath(const std::filesystem::path& filepath);
		void DisableSaveFile();

		void SetMenubarCallback(const std::function<void()>& callback) { m_MenubarCallback = callback; }

		ImGuiContext* GetContext();

	private:
		void _DrawTitlebar();

	private:
		bool m_TitlebarHovered = false;
		std::function<void()> m_MenubarCallback;

		std::unique_ptr<Rendering::Image> m_MinimizeIcon;
		std::unique_ptr<Rendering::Image> m_MaximizeIcon;
		std::unique_ptr<Rendering::Image> m_RestoreIcon;
		std::unique_ptr<Rendering::Image> m_CloseIcon;
	};
}