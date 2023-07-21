#include "pch.hpp"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"


namespace Sphynx::Rendering {
	size_t GetVkFormatSize(VkFormat format) {
		switch (format) {
		default:
			SE_FATAL(Logging::Rendering, "Unsupported texture format");
		case VK_FORMAT_R8G8B8A8_SRGB: return 4;
		case VK_FORMAT_R8G8B8_SRGB: return 3;
		}
	}


	VulkanTexture::VulkanTexture(const TextureSpecification& spec)
		: m_Width(spec.Width), m_Height(spec.Height)
	{
		switch (spec.Format) {
		default:
		case TextureFormat::RGBA:
		case TextureFormat::RGB:
			m_VulkanFormat = VK_FORMAT_R8G8B8A8_SRGB;
			break;
		}

		size_t expectedPixelSize = GetVkFormatSize(m_VulkanFormat);
		size_t expectedDataSize = m_Width * m_Height * expectedPixelSize;

		// Convert RBG to RGBA (RGB isn't supported hardwarewise)
		if (spec.Format == TextureFormat::RGB) {
			size_t suppliedPixelSize = GetVkFormatSize(VK_FORMAT_R8G8B8_SRGB);
			size_t expectedSuppliedDataSize = m_Width * m_Height * suppliedPixelSize;
			SE_ASSERT(expectedSuppliedDataSize == spec.Data.size(), Logging::Rendering, "Wrong data size!");

			m_Data.resize(expectedDataSize);
			for (size_t i = 0; i < m_Width * m_Height; i++) {
				m_Data[i * expectedPixelSize + 0] = spec.Data[i * suppliedPixelSize];
				m_Data[i * expectedPixelSize + 1] = spec.Data[i * suppliedPixelSize + 1];
				m_Data[i * expectedPixelSize + 2] = spec.Data[i * suppliedPixelSize + 2];
				m_Data[i * expectedPixelSize + 3] = 255;
			}
		}
		else {
			SE_ASSERT(expectedDataSize == spec.Data.size(), Logging::Rendering, "Wrong data size!");
			m_Data = spec.Data;
		}
	}

	VulkanTexture::~VulkanTexture() {
		vkDestroyImageView(m_Device, m_View, nullptr);
		m_View = nullptr;
		
		vkDestroyImage(m_Device, m_Image, nullptr);
		m_Image = nullptr;
		
		vkFreeMemory(m_Device, m_Memory, nullptr);
		m_Memory = nullptr;
	}

	void VulkanTexture::UploadToGPU(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VulkanCommandPool& commandPool, VkQueue graphicsQueue) {
		m_Device = logicalDevice;

		VulkanBuffer stagingBuffer(physicalDevice, logicalDevice, m_Data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.Set(m_Data);

		m_Data.clear();

		CreateImage(physicalDevice, m_Device, m_Width, m_Height, m_VulkanFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SHARING_MODE_EXCLUSIVE, m_Image, m_Memory);

		VkCommandBuffer commandBuffer = commandPool.BeginSingleUseCommandbuffer();
		TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_Image);
		CopyBufferToImage(commandBuffer, stagingBuffer.Buffer, m_Image, m_Width, m_Height);
		TransitionImageLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_Image);
		commandPool.EndSingleUseCommandbuffer(commandBuffer, graphicsQueue);

		m_View = CreateImageView(m_Device, m_Image, m_VulkanFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}


	void VulkanTexture::CreateImage(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkSharingMode sharingMode, VkImage& image, VkDeviceMemory& memory, VkImageCreateFlags flags, uint32_t arrayLayers) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = sharingMode;
		imageInfo.flags = flags;

		VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create image");

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		result = vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to allocate memory for image");

		vkBindImageMemory(logicalDevice, image, memory, 0);
	}

	void VulkanTexture::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout, VkImage image) {
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanTexture::CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	VkImageView VulkanTexture::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageViewType type, uint32_t layerCount) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = type;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = layerCount;
		VkImageView imageView;
		VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create image view!");
		return imageView;
	}

	VkSampler VulkanTexture::CreateSampler(VkDevice device) {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1.f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VkSampler sampler;
		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create image sampler");
		return sampler;
	}
}