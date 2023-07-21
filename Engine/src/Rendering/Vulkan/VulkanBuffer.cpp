#include "pch.hpp"
#include "VulkanBuffer.hpp"

namespace Sphynx::Rendering {
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}
		SE_FATAL(Logging::Rendering, "Failed to find memory type");
		return (uint32_t)-1;
	}

	
	VulkanBuffer::VulkanBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: m_Device(logicalDevice)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_Device, &bufferInfo, nullptr, &Buffer);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create Buffer");

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(m_Device, Buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(m_Device, &allocInfo, nullptr, &Memory);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to allocate buffer");

		vkBindBufferMemory(m_Device, Buffer, Memory, 0);
	}

	VulkanBuffer::~VulkanBuffer() {
		vkDestroyBuffer(m_Device, Buffer, nullptr);
		Buffer = VK_NULL_HANDLE;
		vkFreeMemory(m_Device, Memory, nullptr);
		Memory = VK_NULL_HANDLE;
	}

	std::unique_ptr<VulkanBuffer> VulkanBuffer::CreateWithStaging(VulkanCommandPool& commandPool, VkQueue graphicsQueue, VkPhysicalDevice physicalDevice, VkDevice logicalDevice, const std::vector<uint8_t>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
		VulkanBuffer stagingBuffer(physicalDevice, logicalDevice, data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.Set(data);

		if (!(usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT))
			usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		if (!(properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
			properties |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		std::unique_ptr<VulkanBuffer> result = std::make_unique<VulkanBuffer>(physicalDevice, logicalDevice, data.size(), usage, properties);
		VkCommandBuffer commandBuffer = commandPool.BeginSingleUseCommandbuffer();
		stagingBuffer.Copy(commandBuffer, result->Buffer);
		commandPool.EndSingleUseCommandbuffer(commandBuffer, graphicsQueue);
		return result;
	}

	void VulkanBuffer::Set(const std::vector<uint8_t>& data) {
		void* gpuData = nullptr;
		VkResult result = vkMapMemory(m_Device, Memory, 0, data.size(), 0, &gpuData);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to map buffer memory");
		memcpy(gpuData, data.data(), data.size());
		vkUnmapMemory(m_Device, Memory);
	}

	void VulkanBuffer::Get(std::vector<uint8_t>& outData) {
		outData.resize(Size);
		void* gpuData = nullptr;
		VkResult result = vkMapMemory(m_Device, Memory, 0, Size, 0, &gpuData);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to map buffer memory");
		memcpy(outData.data(), gpuData, Size);
		vkUnmapMemory(m_Device, Memory);
	}

	void VulkanBuffer::Copy(VkCommandBuffer commandBuffer, VkBuffer dst) {
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = Size;
		vkCmdCopyBuffer(commandBuffer, Buffer, dst, 1, &copyRegion);
	}
}