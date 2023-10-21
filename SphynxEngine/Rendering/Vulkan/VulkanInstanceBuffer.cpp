#include "pch.hpp"
#include "VulkanInstanceBuffer.hpp"
#include "VulkanContext.hpp"
#include "Rendering/Renderer.hpp"

namespace Sphynx::Rendering {
		VulkanInstanceBuffer::VulkanInstanceBuffer(size_t elementSize, size_t startSize)
        	: m_ElementSize(elementSize), m_Size(std::max(startSize, (size_t)2))
		{
			m_Buffers.resize(VulkanContext::MaxFramesInFlight);
			for (size_t i = 0; i < m_Buffers.size(); i++)
				m_Buffers[i] = std::make_unique<VulkanBuffer>(m_Size * m_ElementSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}

        void VulkanInstanceBuffer::Resize(size_t newSize) {
			Rendering::Renderer::AddBeforeNextRenderCallback([this, newSize]() {
				VulkanContext::LogicalDevice.waitIdle();
				for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
					m_Buffers[i] = std::make_unique<VulkanBuffer>(newSize * m_ElementSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
				}
				m_Size = newSize;
				m_ResizePending = false;
			});
			m_ResizePending = true;
		}

        void VulkanInstanceBuffer::Set(BufferView data) {
			if (data.Size > 0) {
                SE_ASSERT(data.Size % m_ElementSize == 0, Logging::Rendering, "Invalid data, element size of data isn't {}", m_ElementSize);
                
                m_Buffers[VulkanContext::CurrentImage]->Set(BufferView(data.Data, std::min(data.Size, m_Size) * m_ElementSize));
            }
		}

        void VulkanInstanceBuffer::Bind() {
			vk::Buffer buffer = m_Buffers[VulkanContext::CurrentImage]->Buffer;
			std::array<vk::DeviceSize, 1> offsets = { 0 };
			VulkanContext::CommandBuffer.bindVertexBuffers(1, 1, &buffer, offsets.data());
		}
}