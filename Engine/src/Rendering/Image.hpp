#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class VulkanTexture;

	class SE_API Image {
	public:
		Image(const uint8_t* data, size_t size);
		~Image();

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		//VkDescriptorSet
		void* GetDescriptorSet();

	private:
		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;

	private:
		VulkanTexture* m_Texture = nullptr;
	};
}