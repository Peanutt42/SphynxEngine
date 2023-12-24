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