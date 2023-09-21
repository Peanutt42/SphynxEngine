#pragma once

#include "pch.hpp"
#include "VulkanCommandPool.hpp"

namespace Sphynx::Rendering {
	std::optional<uint32> FindMemoryTypeIndex(uint32 typeFilter, vk::MemoryPropertyFlags properties);

	class SE_API VulkanBuffer {
	public:
		vk::Buffer Buffer;
		vk::DeviceMemory Memory;
		vk::DeviceSize Size;

		VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
		~VulkanBuffer();

		static VulkanBuffer* CreateWithStaging(BufferView data, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties = (vk::MemoryPropertyFlags)0);

		void Set(BufferView data);
		void Get(std::vector<byte>& outData);
		void Copy(vk::CommandBuffer commandBuffer, vk::Buffer dst);

	private:
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer(VulkanBuffer&&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(VulkanBuffer&&) = delete;
	};
}