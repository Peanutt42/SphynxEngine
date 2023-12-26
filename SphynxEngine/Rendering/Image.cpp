#include "pch.hpp"
#include "Image.hpp"
#include "Core/Engine.hpp"
#include "Renderer.hpp"
#include "Profiling/Profiling.hpp"

#include "Vulkan/VulkanTexture.hpp"
#include "Vulkan/VulkanContext.hpp"

#include <stb_image.h>

#include <backends/imgui_impl_vulkan.h>

namespace Sphynx::Rendering {
	Image::Image(const std::filesystem::path& filepath) {
		SE_PROFILE_FUNCTION();

		std::string filepathStr = filepath.string();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		uint8* data = stbi_load(filepathStr.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		
		TextureSpecification spec{
			.Width = (uint32)width,
			.Height = (uint32)height,
			.Format = TextureFormat::RGBA
		};

		spec.Data.resize((size_t)width * (size_t)height * STBI_rgb_alpha);
		std::memcpy(spec.Data.data(), data, spec.Data.size());
		stbi_image_free(data);

		m_Texture = new VulkanTexture(spec);
		m_Texture->UploadToGPU();
	}

	Image::Image(BufferView data) {
		SE_PROFILE_FUNCTION();

		int width, height, channels;
		
		uint8* stbi_data = stbi_load_from_memory(data.As<stbi_uc>(), (int)data.Size, &width, &height, &channels, 4);
		size_t stbi_size = (size_t)width * (size_t)height * 4;

		TextureSpecification spec{
			.Width = (uint32)width,
			.Height = (uint32)height,
			.Format = TextureFormat::RGBA
		};

		spec.Data.resize(stbi_size);
		std::memcpy(spec.Data.data(), stbi_data, spec.Data.size());
		stbi_image_free(stbi_data);

		m_Texture = new VulkanTexture(spec);
		m_Texture->UploadToGPU();
	}

	Image::~Image() {
		SE_PROFILE_FUNCTION();

		delete m_Texture;
	}

	uint32 Image::GetWidth() const { return m_Texture->GetWidth(); }
	uint32 Image::GetHeight() const { return m_Texture->GetHeight(); }
	void* Image::GetDescriptorSet() { return m_Texture->GetDescriptorSet(); }
}