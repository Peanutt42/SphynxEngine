#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	class VulkanContext;

	class VulkanCommandPool {
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		VkCommandBuffer BeginRecording(uint32_t frameIndex);
		void EndRecording(uint32_t frameIndex);

		VkCommandBuffer BeginSingleUseCommandbuffer();
		void EndSingleUseCommandbuffer(VkCommandBuffer commandbuffer);

	private:
		VkCommandPool m_Pool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}