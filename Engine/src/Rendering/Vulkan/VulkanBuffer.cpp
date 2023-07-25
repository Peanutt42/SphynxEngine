#include "pch.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(VulkanContext::PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		SE_FATAL(Logging::Rendering, "Failed to find memory type");
		return (uint32_t)-1;
	}

	
	VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: Size(size)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(VulkanContext::LogicalDevice, &bufferInfo, nullptr, &Buffer);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create Buffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(VulkanContext::LogicalDevice, Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(VulkanContext::LogicalDevice, &allocInfo, nullptr, &Memory);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to allocate buffer");

		vkBindBufferMemory(VulkanContext::LogicalDevice, Buffer, Memory, 0);
	}

	VulkanBuffer::~VulkanBuffer() {
		vkDestroyBuffer(VulkanContext::LogicalDevice, Buffer, nullptr);
		Buffer = VK_NULL_HANDLE;
		vkFreeMemory(VulkanContext::LogicalDevice, Memory, nullptr);
		Memory = VK_NULL_HANDLE;
	}

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateWithStaging(BufferView data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
		VulkanBuffer stagingBuffer(data.Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.Set(data);

		if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT))
			usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (!(properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
			properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		std::unique_ptr<VulkanBuffer> result = std::make_unique<VulkanBuffer>(data.Size, usage, properties);
		VkCommandBuffer commandBuffer = VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		stagingBuffer.Copy(commandBuffer, result->Buffer);
		VulkanContext::CommandPool->EndSingleUseCommandbuffer(commandBuffer);
		return result;
	}

	void VulkanBuffer::Set(BufferView data) {
		void* gpuData = nullptr;
		VkResult result = vkMapMemory(VulkanContext::LogicalDevice, Memory, 0, data.Size, 0, &gpuData);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to map buffer memory");
		memcpy(gpuData, data.Data, data.Size);
		vkUnmapMemory(VulkanContext::LogicalDevice, Memory);
	}

	void VulkanBuffer::Get(std::vector<uint8_t>& outData) {
		outData.resize(Size);
		void* gpuData = nullptr;
		VkResult result = vkMapMemory(VulkanContext::LogicalDevice, Memory, 0, Size, 0, &gpuData);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to map buffer memory");
		memcpy(outData.data(), gpuData, Size);
		vkUnmapMemory(VulkanContext::LogicalDevice, Memory);
	}

	void VulkanBuffer::Copy(VkCommandBuffer commandBuffer, VkBuffer dst) {
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = Size;
		vkCmdCopyBuffer(commandBuffer, Buffer, dst, 1, &copyRegion);
	}
}