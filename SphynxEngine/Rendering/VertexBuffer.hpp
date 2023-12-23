#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	enum class VertexAttrib {
		Int,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Mat4,
	};
	struct SE_API VertexLayout {
		std::vector<VertexAttrib> Attributes;

		int GetTotalSize() const;

		VertexLayout& add(VertexAttrib attrib) {
			if (attrib == VertexAttrib::Mat4) {
				Attributes.push_back(VertexAttrib::Vec4);
				Attributes.push_back(VertexAttrib::Vec4);
				Attributes.push_back(VertexAttrib::Vec4);
				Attributes.push_back(VertexAttrib::Vec4);
			}
			else
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