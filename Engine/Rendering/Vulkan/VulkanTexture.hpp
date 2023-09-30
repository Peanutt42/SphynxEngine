#pragma once

#include "pch.hpp"
#include "VulkanCommandPool.hpp"


namespace Sphynx::Rendering {
	enum class TextureFormat {
		RGBA,
		RGB
	};
	
	struct TextureSpecification {
		uint32 Width = 0, Height = 0;
		TextureFormat Format = TextureFormat::RGBA;
		std::vector<byte> Data;
	};

	class VulkanTexture {
	public:
		VulkanTexture(const TextureSpecification& spec);
		~VulkanTexture();

		void UploadToGPU();

		static void CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,vk::MemoryPropertyFlags properties, vk::Image& image,vk::DeviceMemory& memory, vk::ImageCreateFlags flags = (vk::ImageCreateFlags)0, uint32 arrayLayers = 1);
		static void TransitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Image image);
		static void CopyBufferToImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer, vk::Image image, uint32 width, uint32 height);
		static vk::ImageView CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageViewType type = vk::ImageViewType::e2D, uint32 layerCount = 1);
		static vk::Sampler CreateSampler();

		uint32 GetWidth() const { return m_Width; }
		uint32 GetHeight() const { return m_Height; }

		vk::ImageView GetImageView() { return m_View; }

		vk::DescriptorSet GetDescriptorSet() { return m_DescriptorSet; }

	private:
		VulkanTexture(const VulkanTexture&) = delete;
		VulkanTexture(VulkanTexture&&) = delete;
		VulkanTexture& operator=(const VulkanTexture&) = delete;
		VulkanTexture& operator=(VulkanTexture&&) = delete;

	private:
		uint32 m_Width = 0, m_Height = 0;
		std::vector<byte> m_Data;
		vk::Format m_VulkanFormat = vk::Format::eUndefined;

		vk::Image m_Image;
		vk::ImageView m_View;
		vk::DeviceMemory m_Memory;

		vk::Sampler m_Sampler;
		vk::DescriptorSet m_DescriptorSet;
	};
}