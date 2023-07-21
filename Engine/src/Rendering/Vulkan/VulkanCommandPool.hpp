#pragma once

#include "Core/CoreInclude.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	class VulkanContext;

	class VulkanCommandPool {
	public:
		VulkanCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, uint32_t maxFramesInFlight);
		~VulkanCommandPool();

		VkCommandBuffer BeginRecording(uint32_t frameIndex);
		void EndRecording(uint32_t frameIndex);

		VkCommandBuffer BeginSingleUseCommandbuffer();
		void EndSingleUseCommandbuffer(VkCommandBuffer commandbuffer, VkQueue graphicsQueue);

	private:
		VkDevice m_Device = VK_NULL_HANDLE;

		VkCommandPool m_Pool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}