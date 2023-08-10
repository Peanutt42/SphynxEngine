#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanCommandPool.hpp"

namespace Sphynx::Rendering {
	std::optional<uint32_t> FindMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

	class SE_API VulkanBuffer {
	public:
		vk::Buffer Buffer;
		vk::DeviceMemory Memory;
		vk::DeviceSize Size;

		VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
		~VulkanBuffer();

		static VulkanBuffer* CreateWithStaging(BufferView data, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties = (vk::MemoryPropertyFlags)0);

		void Set(BufferView data);
		void Get(std::vector<uint8_t>& outData);
		void Copy(vk::CommandBuffer commandBuffer, vk::Buffer dst);
	};
}