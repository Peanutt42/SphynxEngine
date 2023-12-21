#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class SE_API Framebuffer {
	public:
		Framebuffer(int width, int height);
		~Framebuffer();

		void Bind();

		// Binds the default framebuffer that is rendered to the screen
		static void BindScreen();

		uint32 GetColorTextureID() const { return m_ColorTextureID; }

	private:
		uint32 m_ID = 0;
		int m_Width = 0, m_Height = 0;
		uint32 m_RBO = 0;
		uint32 m_ColorTextureID = 0;
	};
}