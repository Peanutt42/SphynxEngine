#include "pch.hpp"
#include "Image.hpp"
#include "Core/Engine.hpp"
#include "Renderer.hpp"

#include "Vulkan/VulkanContext.hpp"

#include <stb_image.h>

#include <backends/imgui_impl_vulkan.h>

namespace Sphynx::Rendering {
	Image::Image(const uint8_t* data, size_t size) {
		int width, height, channels;
		
		uint8_t* stbi_data = stbi_load_from_memory(data, (int)size, &width, &height, &channels, 4);
		size_t stbi_size = (size_t)width * (size_t)height * 4;

		TextureSpecification spec{
			.Width = (uint32_t)width,
			.Height = (uint32_t)height,
			.Format = TextureFormat::RGBA
		};

		spec.Data.resize(stbi_size);
		std::memcpy(spec.Data.data(), stbi_data, spec.Data.size());
		stbi_image_free(stbi_data);

		m_Texture = std::make_unique<VulkanTexture>(spec);
		m_Texture->UploadToGPU();

		m_Sampler = VulkanTexture::CreateSampler();

		m_DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Sampler, m_Texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Image::~Image() {
		vkDestroySampler(VulkanContext::LogicalDevice, m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}
}