#include "pch.hpp"
#include "VulkanTexture.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanContext.hpp"

#include <backends/imgui_impl_vulkan.h>

namespace Sphynx::Rendering {
	size_t GetVkFormatSize(vk::Format format) {
		switch (format) {
		default:
			SE_FATAL(Logging::Rendering, "Unsupported texture format");
		case vk::Format::eR8G8B8A8Srgb: return 4;
		case vk::Format::eR8G8B8Srgb: return 3;
		}
	}


	VulkanTexture::VulkanTexture(const TextureSpecification& spec)
		: m_Width(spec.Width), m_Height(spec.Height)
	{
		switch (spec.Format) {
		default:
		case TextureFormat::RGBA:
		case TextureFormat::RGB:
			m_VulkanFormat = vk::Format::eR8G8B8A8Srgb;
			break;
		}

		size_t expectedPixelSize = GetVkFormatSize(m_VulkanFormat);
		size_t expectedDataSize = (size_t)m_Width * (size_t)m_Height * expectedPixelSize;

		// Convert RBG to RGBA (RGB isn't supported hardwarewise)
		if (spec.Format == TextureFormat::RGB) {
			size_t suppliedPixelSize = GetVkFormatSize(vk::Format::eR8G8B8Srgb);
			size_t expectedSuppliedDataSize = (size_t)m_Width * (size_t)m_Height * suppliedPixelSize;
			SE_ASSERT(expectedSuppliedDataSize == spec.Data.size(), Logging::Rendering, "Wrong data size!");

			m_Data.resize(expectedDataSize);
			for (size_t i = 0; i < (size_t)m_Width * (size_t)m_Height; i++) {
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
		VulkanContext::LogicalDevice.destroyImageView(m_View, nullptr);
		m_View = nullptr;
		
		VulkanContext::LogicalDevice.destroyImage(m_Image, nullptr);
		m_Image = nullptr;
		
		VulkanContext::LogicalDevice.freeMemory(m_Memory, nullptr);
		m_Memory = nullptr;

		VulkanContext::LogicalDevice.destroySampler(m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}

	void VulkanTexture::UploadToGPU() {
		VulkanBuffer stagingBuffer(m_Data.size(), vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		stagingBuffer.Set(BufferView(m_Data));

		m_Data.clear();

		CreateImage(m_Width, m_Height, m_VulkanFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, m_Image, m_Memory);

		vk::CommandBuffer commandBuffer = VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		TransitionImageLayout(commandBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, m_Image);
		CopyBufferToImage(commandBuffer, stagingBuffer.Buffer, m_Image, m_Width, m_Height);
		TransitionImageLayout(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, m_Image);
		VulkanContext::CommandPool->EndSingleUseCommandbuffer(commandBuffer);

		m_View = CreateImageView(m_Image, m_VulkanFormat, vk::ImageAspectFlagBits::eColor);
	
		m_Sampler = VulkanTexture::CreateSampler();

		m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}


	void VulkanTexture::CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& memory, vk::ImageCreateFlags flags, uint32 arrayLayers) {
		vk::ImageCreateInfo imageInfo{};
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = usage;
		imageInfo.samples = vk::SampleCountFlagBits::e1;
		imageInfo.sharingMode = VulkanContext::SharingMode;
		imageInfo.flags = flags;

		vk::Result result = VulkanContext::LogicalDevice.createImage(&imageInfo, nullptr, &image);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create image");

		vk::MemoryRequirements memRequirements;
		VulkanContext::LogicalDevice.getImageMemoryRequirements(image, &memRequirements);

		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size;
		std::optional<uint32> memoryTypeIndex = FindMemoryTypeIndex(memRequirements.memoryTypeBits, properties);
		SE_ASSERT(memoryTypeIndex.has_value(), Logging::Rendering, "Failed to get memory type index");
		allocInfo.memoryTypeIndex = *memoryTypeIndex;

		result = VulkanContext::LogicalDevice.allocateMemory(&allocInfo, nullptr, &memory);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to allocate memory for image");

		VulkanContext::LogicalDevice.bindImageMemory(image, memory, 0);
	}

	void VulkanTexture::TransitionImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::Image image) {
		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
			barrier.srcAccessMask = (vk::AccessFlagBits)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			barrier.srcAccessMask = (vk::AccessFlagBits)0;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal&& newLayout == vk::ImageLayout::eTransferSrcOptimal) {
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(
			sourceStage, destinationStage,
			(vk::DependencyFlags)0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanTexture::CopyBufferToImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer, vk::Image image, uint32 width, uint32 height) {
		vk::BufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = vk::Offset3D{ 0, 0, 0 };
		region.imageExtent = vk::Extent3D{
			width,
			height,
			1
		};

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	}

	vk::ImageView VulkanTexture::CreateImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, vk::ImageViewType type, uint32 layerCount) {
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.image = image;
		viewInfo.viewType = type;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = layerCount;
		vk::ImageView imageView;
		vk::Result result = VulkanContext::LogicalDevice.createImageView(&viewInfo, nullptr, &imageView);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create image view!");
		return imageView;
	}

	vk::Sampler VulkanTexture::CreateSampler() {
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = vk::Filter::eLinear;
		samplerInfo.minFilter = vk::Filter::eLinear;
		samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 1.f;
		samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = vk::CompareOp::eAlways;
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		vk::Sampler sampler;
		vk::Result result = VulkanContext::LogicalDevice.createSampler(&samplerInfo, nullptr, &sampler);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create image sampler");
		return sampler;
	}
}