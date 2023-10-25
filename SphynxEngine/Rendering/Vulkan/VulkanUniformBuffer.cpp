#include "pch.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanUniformBuffer::VulkanUniformBuffer(size_t size)
		: Size(size)
	{
		Buffers.resize(VulkanContext::MaxFramesInFlight);
		MappedMemories.resize(VulkanContext::MaxFramesInFlight);

		for (size_t i = 0; i < Buffers.size(); i++) {
			Buffers[i] = std::make_shared<VulkanBuffer>(Size, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
			vk::Result result = VulkanContext::LogicalDevice.mapMemory(Buffers[i]->Memory, 0, Size, (vk::MemoryMapFlags)0, &MappedMemories[i]);
			SE_ASSERT(result == vk::Result::eSuccess, "Failed to map memory for uniform buffer");
		}
	}

	VulkanUniformBuffer::~VulkanUniformBuffer() {
		for (size_t i = 0; i < Buffers.size(); i++)
			VulkanContext::LogicalDevice.unmapMemory(Buffers[i]->Memory);

		MappedMemories.clear();
		Buffers.clear();
	}
}