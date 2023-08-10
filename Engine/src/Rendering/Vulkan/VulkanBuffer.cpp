#include "pch.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	std::optional<uint32_t> FindMemoryTypeIndex(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
		vk::PhysicalDeviceMemoryProperties memProperties = VulkanContext::PhysicalDevice.getMemoryProperties();
		
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) &&
				memProperties.memoryTypes[i].propertyFlags & properties)
				return i;
		}
		return {};
	}

	
	VulkanBuffer::VulkanBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
		: Size(size)
	{
		SE_PROFILE_FUNCTION();

		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::Result result = VulkanContext::LogicalDevice.createBuffer(&bufferInfo, nullptr, &Buffer);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create Buffer");

		vk::MemoryRequirements memRequirements;
		VulkanContext::LogicalDevice.getBufferMemoryRequirements(Buffer, &memRequirements);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		std::optional<uint32_t> memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);
		SE_ASSERT(memoryTypeIndex.has_value(), Logging::Rendering, "Failed to find memory type");
		allocInfo.memoryTypeIndex = *memoryTypeIndex;

		result = VulkanContext::LogicalDevice.allocateMemory(&allocInfo, nullptr, &Memory);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to allocate buffer");

		VulkanContext::LogicalDevice.bindBufferMemory(Buffer, Memory, 0);
	}

	VulkanBuffer::~VulkanBuffer() {
		SE_PROFILE_FUNCTION();

		VulkanContext::LogicalDevice.destroyBuffer(Buffer, nullptr);
		Buffer = VK_NULL_HANDLE;
		VulkanContext::LogicalDevice.freeMemory(Memory, nullptr);
		Memory = VK_NULL_HANDLE;
	}

	VulkanBuffer* VulkanBuffer::CreateWithStaging(BufferView data, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
		SE_PROFILE_FUNCTION();

		VulkanBuffer stagingBuffer(data.Size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		stagingBuffer.Set(data);

		// if they aren't set already
		usage |= vk::BufferUsageFlagBits::eTransferDst;
		properties |= vk::MemoryPropertyFlagBits::eDeviceLocal;

		VulkanBuffer* result = new VulkanBuffer(data.Size, usage, properties);
		vk::CommandBuffer commandBuffer = VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		stagingBuffer.Copy(commandBuffer, result->Buffer);
		VulkanContext::CommandPool->EndSingleUseCommandbuffer(commandBuffer);
		return result;
	}

	void VulkanBuffer::Set(BufferView data) {
		SE_PROFILE_FUNCTION();

		void* gpuData = nullptr;
		vk::Result result = VulkanContext::LogicalDevice.mapMemory(Memory, 0, data.Size, (vk::MemoryMapFlags)0, &gpuData);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to map buffer memory");
		memcpy(gpuData, data.Data, data.Size);
		VulkanContext::LogicalDevice.unmapMemory(Memory);
	}

	void VulkanBuffer::Get(std::vector<uint8_t>& outData) {
		SE_PROFILE_FUNCTION();

		outData.resize(Size);
		void* gpuData = nullptr;
		vk::Result result = VulkanContext::LogicalDevice.mapMemory(Memory, 0, Size, (vk::MemoryMapFlags)0, &gpuData);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to map buffer memory");
		memcpy(outData.data(), gpuData, Size);
		VulkanContext::LogicalDevice.unmapMemory(Memory);
	}

	void VulkanBuffer::Copy(vk::CommandBuffer commandBuffer, vk::Buffer dst) {
		SE_PROFILE_FUNCTION();

		vk::BufferCopy copyRegion{};
		copyRegion.size = Size;
		commandBuffer.copyBuffer(Buffer, dst, 1, &copyRegion);
	}
}