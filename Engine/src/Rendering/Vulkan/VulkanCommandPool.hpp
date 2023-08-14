#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.hpp>

namespace Sphynx::Rendering {
	class VulkanContext;

	class VulkanCommandPool {
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		vk::CommandBuffer BeginRecording(uint32 frameIndex);
		void EndRecording(uint32 frameIndex);

		vk::CommandBuffer BeginSingleUseCommandbuffer();
		void EndSingleUseCommandbuffer(vk::CommandBuffer commandbuffer);

	private:
		vk::CommandPool m_Pool;
		std::vector<vk::CommandBuffer> m_CommandBuffers;
	};
}