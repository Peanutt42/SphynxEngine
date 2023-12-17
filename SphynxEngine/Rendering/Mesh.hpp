#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	enum class VertexAttrib {
		Vec3,
	};

	inline static int GetVertexAttribElementType(VertexAttrib attrib);
	inline static int GetVertexAttribElementCount(VertexAttrib attrib);
	inline static size_t GetVertexAttribSize(VertexAttrib attrib);

	struct SE_API VertexLayout {
		std::vector<VertexAttrib> Attributes;

		VertexLayout& add(VertexAttrib attrib) {
			Attributes.push_back(attrib);
			return *this;
		}
	};

	class SE_API Mesh {
	public:
		Mesh(BufferView vertices, const VertexLayout& vertexLayout, const std::vector<uint32>& indices);

		template<typename T>
		Mesh(const std::vector<T>& vertices, const std::vector<uint32>& indices) : Mesh(BufferView(vertices), T::GetVertexLayout(), indices) {}

		~Mesh();

		void Draw();

	private:

		uint32 m_VBO = 0, m_VAO = 0, m_EBO = 0;
		size_t m_IndexCount = 0;
	};
}