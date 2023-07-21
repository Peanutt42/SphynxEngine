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

		void UploadToGPU(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VulkanCommandPool& commandPool, VkQueue graphicsQueue);

		static void CreateImage(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode, VkImage& image, VkDeviceMemory& memory, VkImageCreateFlags flags = 0, uint32_t arrayLayers = 1);
		static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image);
		static void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
		static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType type = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1);
		static VkSampler CreateSampler(VkDevice device);


	private:
		uint32_t m_Width = 0, m_Height = 0;
		std::vector<uint8_t> m_Data;
		VkFormat m_VulkanFormat = VK_FORMAT_UNDEFINED;

		VkDevice m_Device = VK_NULL_HANDLE;

		VkImage m_Image = VK_NULL_HANDLE;
		VkImageView m_View = VK_NULL_HANDLE;
		VkDeviceMemory m_Memory = VK_NULL_HANDLE;
	};
}