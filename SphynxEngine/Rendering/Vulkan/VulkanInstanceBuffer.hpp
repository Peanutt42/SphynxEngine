#pragma once

#include "pch.hpp"
#include "VulkanBuffer.hpp"
#include "Core/Engine.hpp"

namespace Sphynx::Rendering {
	class VulkanInstanceBuffer {
	public:
		VulkanInstanceBuffer(size_t elementSize, size_t startSize = 2);
		
		void Resize(size_t newSize);

		void Set(BufferView data);

		void Bind();

		size_t GetSize() const { return m_Size; }

	private:
		VulkanInstanceBuffer(const VulkanInstanceBuffer&) = delete;
		VulkanInstanceBuffer(VulkanInstanceBuffer&&) = delete;
		VulkanInstanceBuffer& operator=(const VulkanInstanceBuffer&) = delete;
		VulkanInstanceBuffer& operator=(VulkanInstanceBuffer&&) = delete;

	private:
		const size_t m_ElementSize = 0;
		std::vector<std::unique_ptr<VulkanBuffer>> m_Buffers;
		size_t m_Size = 0;
		bool m_ResizePending = false;
	};
}