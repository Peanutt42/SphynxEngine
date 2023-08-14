#include "pch.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanDevice.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanCommandPool::VulkanCommandPool() {
		m_CommandBuffers.resize(VulkanContext::MaxFramesInFlight, VK_NULL_HANDLE);

		VulkanQueueFamilyIndices queueFamilyIndices(VulkanContext::PhysicalDevice);

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

		vk::Result result = VulkanContext::LogicalDevice.createCommandPool(&poolInfo, nullptr, &m_Pool);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create commandPool");
		

		vk::CommandBufferAllocateInfo commandBufferAllocInfo{};
		commandBufferAllocInfo.commandPool = m_Pool;
		commandBufferAllocInfo.level = vk::CommandBufferLevel::ePrimary;
		commandBufferAllocInfo.commandBufferCount = (uint32)m_CommandBuffers.size();

		result = VulkanContext::LogicalDevice.allocateCommandBuffers(&commandBufferAllocInfo, m_CommandBuffers.data());
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to allocate commandBuffers");
	}

	VulkanCommandPool::~VulkanCommandPool() {
		VulkanContext::LogicalDevice.destroyCommandPool(m_Pool, nullptr);
		m_Pool = VK_NULL_HANDLE;
	}

	vk::CommandBuffer VulkanCommandPool::BeginRecording(uint32 frameIndex) {
		SE_ASSERT(frameIndex <= m_CommandBuffers.size(), Logging::Rendering, "Invalid frameIndex");

		m_CommandBuffers[frameIndex].reset();

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = {}; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		vk::Result result = m_CommandBuffers[frameIndex].begin(&beginInfo);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to begin recording commandBuffer");

		return m_CommandBuffers[frameIndex];
	}

	void VulkanCommandPool::EndRecording(uint32 frameIndex) {
		SE_ASSERT(frameIndex <= m_CommandBuffers.size(), Logging::Rendering, "Invalid frameIndex");

		m_CommandBuffers[frameIndex].end();
	}

	vk::CommandBuffer VulkanCommandPool::BeginSingleUseCommandbuffer() {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandPool = m_Pool;
		allocInfo.commandBufferCount = 1;

		vk::CommandBuffer commandBuffer;
		vk::Result result = VulkanContext::LogicalDevice.allocateCommandBuffers(&allocInfo, &commandBuffer);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to allocate commandBuffers");

		vk::CommandBufferBeginInfo beginInfo{};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		result = commandBuffer.begin(&beginInfo);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to begin single use commmand buffer");

		return commandBuffer;
	}

	void VulkanCommandPool::EndSingleUseCommandbuffer(vk::CommandBuffer commandbuffer) {
		commandbuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandbuffer;

		vk::Result result = VulkanContext::GraphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		SE_ASSERT(result == vk::Result::eSuccess, "Failed to sumit single use commandbuffer");

		VulkanContext::GraphicsQueue.waitIdle();

		VulkanContext::LogicalDevice.freeCommandBuffers(m_Pool, 1, &commandbuffer);
	}
}