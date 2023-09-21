#pragma once

#include "pch.hpp"
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
		VulkanImGuiHelper(const VulkanImGuiHelper&) = delete;
		VulkanImGuiHelper(VulkanImGuiHelper&&) = delete;
		VulkanImGuiHelper& operator=(const VulkanImGuiHelper&) = delete;
		VulkanImGuiHelper& operator=(VulkanImGuiHelper&&) = delete;

		void _DrawTitlebar();

	private:
		bool m_TitlebarHovered = false;
		std::function<void()> m_MenubarCallback;

		std::unique_ptr<Rendering::Image> m_MinimizeIcon;
		std::unique_ptr<Rendering::Image> m_MaximizeIcon;
		std::unique_ptr<Rendering::Image> m_RestoreIcon;
		std::unique_ptr<Rendering::Image> m_CloseIcon;

		float m_MinimizeOpacity = 0.f;
		float m_MaximizeOpacity = 0.f;
		float m_CloseOpacity = 0.f;
	};
}