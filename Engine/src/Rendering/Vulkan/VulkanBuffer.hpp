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

		static std::unique_ptr<VulkanBuffer> CreateWithStaging(const std::vector<uint8_t>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties = 0);

		void Set(const std::vector<uint8_t>& data);
		void Get(std::vector<uint8_t>& outData);
		void Copy(VkCommandBuffer commandBuffer, VkBuffer dst);
	};
}