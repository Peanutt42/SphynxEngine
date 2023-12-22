#include "pch.hpp"
#include "VertexArray.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	int GetVertexAttribElementType(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int: return GL_INT;
		case VertexAttrib::Float:
		case VertexAttrib::Vec2:
		case VertexAttrib::Vec3: return GL_FLOAT;
		}
	}

	int GetVertexAttribElementCount(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int:
		case VertexAttrib::Float: return 1;
		case VertexAttrib::Vec2: return 2;
		case VertexAttrib::Vec3: return 3;
		}
	}

	size_t GetVertexAttribSize(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int: return sizeof(int);
		case VertexAttrib::Float: return sizeof(float);
		case VertexAttrib::Vec2: return 2 * sizeof(float);
		case VertexAttrib::Vec3: return 3 * sizeof(float);
		}
	}

	VertexArray::VertexArray() {
		glGenVertexArrays(1, &m_ID);
	}

	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_ID);
		m_ID = 0;
	}

	void VertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) {
		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();

		size_t total_vertex_size = 0;
		for (const auto& attrib : layout.Attributes)
			total_vertex_size += GetVertexAttribSize(attrib);

		byte* offset = nullptr;
		for (int i = 0; i < layout.Attributes.size(); i++) {
			const auto& attrib = layout.Attributes[i];

			glVertexAttribPointer(i, GetVertexAttribElementCount(attrib), GetVertexAttribElementType(attrib), GL_FALSE, total_vertex_size, offset);
			glEnableVertexAttribArray(i);

			offset += GetVertexAttribSize(attrib);
		}
		
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) {
		glBindVertexArray(m_ID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

	void VertexArray::Bind() {
		glBindVertexArray(m_ID);
	}

	void VertexArray::Unbind() {
		glBindVertexArray(0);
	}
}