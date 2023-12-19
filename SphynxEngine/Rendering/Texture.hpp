#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class SE_API Texture {
	public:
		Texture(const std::filesystem::path& filepath);
		~Texture();

		void Bind(uint32 slot = 0);

	private:
		uint32 m_ID = 0;
	};
}