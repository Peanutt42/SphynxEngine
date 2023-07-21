#include "pch.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanCommandPool::VulkanCommandPool(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkSurfaceKHR surface, uint32_t maxFramesInFlight)
		: m_Device(logicalDevice)
	{
		m_CommandBuffers.resize(maxFramesInFlight, VK_NULL_HANDLE);

		VulkanQueueFamilyIndices queueFamilyIndices(physicalDevice, surface);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

		VkResult result = vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Pool);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create commandPool");
		

		VkCommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocInfo.commandPool = m_Pool;
		commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

		result = vkAllocateCommandBuffers(m_Device, &commandBufferAllocInfo, m_CommandBuffers.data());
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to allocate commandBuffers");
	}

	VulkanCommandPool::~VulkanCommandPool() {
		vkDestroyCommandPool(m_Device, m_Pool, nullptr);
		m_Pool = VK_NULL_HANDLE;
	}

	VkCommandBuffer VulkanCommandPool::BeginRecording(uint32_t frameIndex) {
		SE_ASSERT(frameIndex <= m_CommandBuffers.size(), Logging::Rendering, "Invalid frameIndex");

		vkResetCommandBuffer(m_CommandBuffers[frameIndex], 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		VkResult result = vkBeginCommandBuffer(m_CommandBuffers[frameIndex], &beginInfo);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to begin recording commandBuffer");

		return m_CommandBuffers[frameIndex];
	}

	void VulkanCommandPool::EndRecording(uint32_t frameIndex) {
		SE_ASSERT(frameIndex <= m_CommandBuffers.size(), Logging::Rendering, "Invalid frameIndex");

		VkResult result = vkEndCommandBuffer(m_CommandBuffers[frameIndex]);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to end recording commandBuffer");
	}

	VkCommandBuffer VulkanCommandPool::BeginSingleUseCommandbuffer() {
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Pool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void VulkanCommandPool::EndSingleUseCommandbuffer(VkCommandBuffer commandbuffer, VkQueue graphicsQueue) {
		vkEndCommandBuffer(commandbuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandbuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(m_Device, m_Pool, 1, &commandbuffer);
	}
}