#pragma once

#include "Core/CoreInclude.hpp"
#include "Vulkan/VulkanTexture.hpp"

namespace Sphynx::Rendering {
	class SE_API Image {
	public:
		Image(const uint8_t* data, size_t size);
		~Image();

		uint32_t GetWidth() const { return m_Texture->GetWidth(); }
		uint32_t GetHeight() const { return m_Texture->GetHeight(); }

		VkDescriptorSet GetDescriptorSet() { return m_DescriptorSet; }

	private:
		std::unique_ptr<VulkanTexture> m_Texture;
		VkSampler m_Sampler = VK_NULL_HANDLE;
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
	};
}