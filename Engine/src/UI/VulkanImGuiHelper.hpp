#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Vulkan/VulkanContext.hpp"

namespace Sphynx::UI {
	class VulkanImGuiHelper {
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
	};
}