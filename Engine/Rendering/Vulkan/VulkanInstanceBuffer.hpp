#pragma once

#include "pch.hpp"
#include "VulkanBuffer.hpp"
#include "Core/Engine.hpp"
#include "Rendering/Renderer.hpp"

namespace Sphynx::Rendering {
	template<typename T>
	class VulkanInstanceBuffer {
	public:
		VulkanInstanceBuffer(size_t maxFramesInFlight, size_t startSize = 2)
			: m_MaxFramesInFlight(maxFramesInFlight), m_Size(std::max(startSize, (size_t)2))
		{
			m_Buffers.resize(m_MaxFramesInFlight);
			for (size_t i = 0; i < m_Buffers.size(); i++)
				m_Buffers[i] = std::make_unique<VulkanBuffer>(m_Size * sizeof(T), vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		}
		
		void Resize(size_t newSize) {
			Engine::Renderer().AddBeforeNextRenderCallback([this, newSize]() {
				VulkanContext::LogicalDevice.waitIdle();
				for (size_t i = 0; i < m_MaxFramesInFlight; i++) {
					m_Buffers[i] = std::make_unique<VulkanBuffer>(newSize * sizeof(T), vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
				}
				m_Size = newSize;
				m_ResizePending = false;
			});
			m_ResizePending = true;
		}

		void Set(const std::vector<T>& data) {
			if (data.size() > 0)
				m_Buffers[VulkanContext::CurrentImage]->Set(BufferView(data.data(), std::min(data.size(), m_Size) * sizeof(T)));
		}

		void Bind() {
			vk::Buffer buffer = m_Buffers[VulkanContext::CurrentImage]->Buffer;
			std::array<vk::DeviceSize, 1> offsets = { 0 };
			VulkanContext::CommandBuffer.bindVertexBuffers(1, 1, &buffer, offsets.data());
		}

		size_t GetSize() const { return m_Size; }

	private:
		VulkanInstanceBuffer(const VulkanInstanceBuffer&) = delete;
		VulkanInstanceBuffer(VulkanInstanceBuffer&&) = delete;
		VulkanInstanceBuffer& operator=(const VulkanInstanceBuffer&) = delete;
		VulkanInstanceBuffer& operator=(VulkanInstanceBuffer&&) = delete;

	private:
		std::vector<std::unique_ptr<VulkanBuffer>> m_Buffers;
		size_t m_Size = 0;
		size_t m_MaxFramesInFlight = 0;
		bool m_ResizePending = false;
	};
}