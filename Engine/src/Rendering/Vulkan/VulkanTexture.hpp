#pragma once

#include "Core/CoreInclude.hpp"
#include "VulkanCommandPool.hpp"


namespace Sphynx::Rendering {
	enum class TextureFormat {
		RGBA,
		RGB
	};
	
	struct TextureSpecification {
		uint32_t Width = 0, Height = 0;
		TextureFormat Format = TextureFormat::RGBA;
		std::vector<uint8_t> Data;
	};

	class SE_API VulkanTexture {
	public:
		VulkanTexture(const TextureSpecification& spec);
		~VulkanTexture();

		void UploadToGPU();

		static void CreateImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,vk::MemoryPropertyFlags properties, vk::Image& image,vk::DeviceMemory& memory, vk::ImageCreateFlags flags = (vk::ImageCreateFlags)0, uint32_t arrayLayers = 1);
		static void TransitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Image image);
		static void CopyBufferToImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
		static vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageViewType type = vk::ImageViewType::e2D, uint32_t layerCount = 1);
		static vk::Sampler CreateSampler();

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		vk::ImageView GetImageView() { return m_View; }

		vk::DescriptorSet GetDescriptorSet() { return m_DescriptorSet; }

	private:
		uint32_t m_Width = 0, m_Height = 0;
		std::vector<uint8_t> m_Data;
		vk::Format m_VulkanFormat = vk::Format::eUndefined;

		vk::Image m_Image;
		vk::ImageView m_View;
		vk::DeviceMemory m_Memory;

		vk::Sampler m_Sampler;
		vk::DescriptorSet m_DescriptorSet;
	};
}