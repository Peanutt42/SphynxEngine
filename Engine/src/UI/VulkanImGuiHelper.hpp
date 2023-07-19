#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Vulkan/VulkanContext.hpp"
#include <imgui/imgui.h>

namespace Sphynx::UI {
	class SE_API VulkanImGuiHelper {
	public:
		VulkanImGuiHelper(Rendering::VulkanContext& context);
		~VulkanImGuiHelper();

		void Begin();
		void End();

		void EnableDocking();
		void BeginDockspace(const std::string& name);
		void EndDockspace();

		void Render(VkCommandBuffer cmd);

		void SetSaveFilepath(const std::filesystem::path& filepath);
		void DisableSaveFile();

		ImGuiContext* GetContext();
	};
}