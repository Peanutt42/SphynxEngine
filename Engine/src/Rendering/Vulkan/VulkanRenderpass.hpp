#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanSwapChain.hpp"

namespace Sphynx::Rendering {
	class VulkanRenderpass {
	public:
		VulkanRenderpass(VkDevice device, VulkanSwapChain& swapchain);
		~VulkanRenderpass();

		VkRenderPass GetHandle() { return m_Renderpass; }

		void Begin(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void End(VkCommandBuffer commandBuffer);

	private:
		VkDevice m_Device = VK_NULL_HANDLE;
		VulkanSwapChain& m_Swapchain;
		VkRenderPass m_Renderpass = VK_NULL_HANDLE;
	};
}