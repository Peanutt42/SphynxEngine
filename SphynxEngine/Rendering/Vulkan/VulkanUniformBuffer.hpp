#pragma once

#include "pch.hpp"
#include "VulkanBuffer.hpp"

namespace Sphynx::Rendering {
	class VulkanUniformBuffer {
	public:
		std::vector<std::shared_ptr<VulkanBuffer>> Buffers;
		std::vector<void*> MappedMemories;
		size_t Size = 0;

		template<typename T>
		VulkanUniformBuffer() : VulkanUniformBuffer(sizeof(T)) {}

		VulkanUniformBuffer(size_t size);
		~VulkanUniformBuffer();
		
		template<typename T>
		void Update(const T& data) {
			if (sizeof(T) != Size) {
				SE_ERR(Logging::Rendering, "Can't update a uniform buffer with a diffrent size!");
				return;
			}

			for (void* mappedMemory : MappedMemories)
				std::memcpy(mappedMemory, &data, sizeof(data));
		}

		vk::Buffer GetBuffer(int index) const {
			return Buffers[index]->Buffer;
		}

		std::vector<vk::Buffer> GetBuffers() const {
			std::vector<vk::Buffer> buffers(Buffers.size());
			for (size_t i = 0; i < Buffers.size(); i++)
				buffers[i] = Buffers[i]->Buffer;
			return buffers;
		}

	private:
		VulkanUniformBuffer(const VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer(VulkanUniformBuffer&&) = delete;
		VulkanUniformBuffer& operator=(const VulkanUniformBuffer&) = delete;
		VulkanUniformBuffer& operator=(VulkanUniformBuffer&&) = delete;
	};
} 