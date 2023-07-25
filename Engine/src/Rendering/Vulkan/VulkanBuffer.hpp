#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanCommandPool.hpp"

namespace Sphynx::Rendering {
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	class SE_API VulkanBuffer {
	public:
		VkBuffer Buffer;
		VkDeviceMemory Memory;
		VkDeviceSize Size;

		VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~VulkanBuffer();

		static std::unique_ptr<VulkanBuffer> CreateWithStaging(BufferView data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties = 0);

		void Set(BufferView data);
		void Get(std::vector<uint8_t>& outData);
		void Copy(VkCommandBuffer commandBuffer, VkBuffer dst);
	};
}