#pragma once

#include "pch.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	template<typename T>
	class VulkanUniformBuffer {
	public:
		std::vector<std::shared_ptr<VulkanBuffer>> Buffers;
		std::vector<void*> MappedMemories;

		VulkanUniformBuffer() {
			Buffers.resize(VulkanContext::MaxFramesInFlight);
			MappedMemories.resize(VulkanContext::MaxFramesInFlight);

			for (size_t i = 0; i < Buffers.size(); i++) {
				Buffers[i] = std::make_shared<VulkanBuffer>(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				vkMapMemory(VulkanContext::LogicalDevice, Buffers[i]->Memory, 0, sizeof(T), 0, &MappedMemories[i]);
			}
		}

		~VulkanUniformBuffer() {
			for (size_t i = 0; i < Buffers.size(); i++) {
				vkUnmapMemory(VulkanContext::LogicalDevice, Buffers[i]->Memory);
			}
			MappedMemories.clear();
			Buffers.clear();
		}
		
		void Update(const T& data) {
			for (void* mappedMemory : MappedMemories)
				std::memcpy(mappedMemory, &data, sizeof(data));
		}

		std::vector<VkBuffer> GetBuffers() const {
			std::vector<VkBuffer> buffers(Buffers.size());
			for (size_t i = 0; i < Buffers.size(); i++)
				buffers[i] = Buffers[i]->Buffer;
			return buffers;
		}
	};
} 