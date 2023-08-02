#include "pch.hpp"
#include "Image.hpp"
#include "Core/Engine.hpp"
#include "Renderer.hpp"

#include "Vulkan/VulkanTexture.hpp"
#include "Vulkan/VulkanContext.hpp"

#include <stb_image.h>

#include <backends/imgui_impl_vulkan.h>

namespace Sphynx::Rendering {
	struct Image::ImplData {
		std::unique_ptr<VulkanTexture> Texture;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
	};

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

		m_Data = new ImplData();

		m_Data->Texture = std::make_unique<VulkanTexture>(spec);
		m_Data->Texture->UploadToGPU();

		m_Data->Sampler = VulkanTexture::CreateSampler();

		m_Data->DescriptorSet = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(m_Data->Sampler, m_Data->Texture->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	Image::~Image() {
		vkDestroySampler(VulkanContext::LogicalDevice, m_Data->Sampler, nullptr);
		m_Data->Sampler = VK_NULL_HANDLE;

		delete m_Data;
	}

	uint32_t Image::GetWidth() const { return m_Data->Texture->GetWidth(); }
	uint32_t Image::GetHeight() const { return m_Data->Texture->GetHeight(); }
	void* Image::GetDescriptorSet() { return m_Data->DescriptorSet; }
}