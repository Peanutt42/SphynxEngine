#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class SE_API Texture {
	public:
		Texture(const std::filesystem::path& filepath);
		Texture(uint32 id) : m_ID(id) {}
		~Texture();

		void Bind(uint32 slot = 0);

		static void Bind(uint32 id, uint32 slot);
			
		uint32 GetID() const { return m_ID; }

	private:
		uint32 m_ID = 0;
	};
}