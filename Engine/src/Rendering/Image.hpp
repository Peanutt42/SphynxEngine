#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class VulkanTexture;

	class SE_API Image {
	public:
		Image(BufferView data);
		~Image();

		uint32 GetWidth() const;
		uint32 GetHeight() const;

		//VkDescriptorSet
		void* GetDescriptorSet();

	private:
		Image(const Image&) = delete;
		Image(Image&&) = delete;
		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = delete;

	private:
		VulkanTexture* m_Texture = nullptr;
	};
}