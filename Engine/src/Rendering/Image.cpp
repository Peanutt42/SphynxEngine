#include "pch.hpp"
#include "Image.hpp"
#include "Core/Engine.hpp"
#include "Renderer.hpp"

#include <stb_image.h>

#include <imgui/imgui_impl_vulkan.h>

namespace Sphynx::Rendering {
	Image::Image(const uint8_t* data, size_t size) {
		int width, height, channels;
		
		uint8_t* stbi_data = stbi_load_from_memory(data, (int)size, &width, &height, &channels, 4);
		uint64_t stbi_size = width * height * 4;

		TextureSpecification spec{
			.Width = (uint32_t)width,
			.Height = (uint32_t)height,
			.Format = TextureFormat::RGBA
		};

		spec.Data.resize(stbi_size);
		std::memcpy(spec.Data.data(), stbi_data, spec.Data.size());
		stbi_image_free(stbi_data);

		m_Texture = std::make_unique<VulkanTexture>(spec);
		VulkanContext& context = Engine::Renderer().GetVulkanContext();
		m_Texture->UploadToGPU(context.GetPhysicalDevice(), context.GetDevice(), context.GetCommandPool(), context.GetGraphicsQueue());

		// Create sampler:
		{
			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			info.minLod = -1000;
			info.maxLod = 1000;
			info.maxAnisotropy = 1.0f;
			VkResult result = vkCreateSampler(context.GetDevice(), &info, nullptr, &m_Sampler);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create image sampler");
		}

		m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_Texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Image::~Image() {
		VulkanContext& context = Engine::Renderer().GetVulkanContext();
		vkDestroySampler(context.GetDevice(), m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}
}