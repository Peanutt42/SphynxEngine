#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	enum class VertexAttrib {
		Int,
		Float,
		Vec2,
		Vec3,
	};
	struct SE_API VertexLayout {
		std::vector<VertexAttrib> Attributes;

		VertexLayout& add(VertexAttrib attrib) {
			Attributes.push_back(attrib);
			return *this;
		}
	};

	class SE_API VertexBuffer {
	public:
		VertexBuffer(uint32 size, const VertexLayout& layout);
		VertexBuffer(BufferView vertices, const VertexLayout& layout);
		~VertexBuffer();

		void SetData(BufferView vertices);

		void Bind();
		void Unbind();

		const VertexLayout& GetLayout() const { return m_Layout; }

	private:
		uint32 m_ID = 0;
		VertexLayout m_Layout;
	};
}