#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	class SE_API IndexBuffer {
	public:
		IndexBuffer(const std::vector<uint32>& indices);
		~IndexBuffer();

		void Bind();
		void Unbind();

		uint32 GetIndexCount() const { return m_IndexCount; }

	private:
		uint32 m_ID = 0;
		uint32 m_IndexCount = 0;
	};
}