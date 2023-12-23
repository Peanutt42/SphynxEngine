#include "pch.hpp"
#include "Texture.hpp"
#include "Profiling/Profiling.hpp"

#include <glad/glad.h>
#include <stb_image.h>

namespace Sphynx::Rendering {
	Texture::Texture(const std::filesystem::path& filepath) {
		SE_PROFILE_FUNCTION();

		int width = 0, height = 0, channels = 0;
		GLenum format = GL_RGBA, internalFormat = GL_RGBA8;
		std::string filepathStr = filepath.string();
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(filepathStr.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		
		if (data) {
			glGenTextures(1, &m_ID);
			glBindTexture(GL_TEXTURE_2D, m_ID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			stbi_image_free(data);
		}
		else
			SE_ERR(Logging::Rendering, "Failed to load texture {}", filepathStr);
	}

	Texture::~Texture() {
		SE_PROFILE_FUNCTION();

		glDeleteTextures(1, &m_ID);
	}

	void Texture::Bind(uint32 slot) {
		SE_PROFILE_FUNCTION();

		Texture::Bind(m_ID, slot);
	}

	void Texture::Bind(uint32 id, uint32 slot) {
		SE_PROFILE_FUNCTION();

#ifndef GL_TEXTURE31
#error "there should be at least 32 texture slots"
#endif

		if (slot < 32) {
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, id);
		}
	}
}