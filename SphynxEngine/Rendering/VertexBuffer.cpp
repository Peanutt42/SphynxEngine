#include "pch.hpp"
#include "VertexBuffer.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	VertexBuffer::VertexBuffer(uint32 size, const VertexLayout& layout) : VertexBuffer(BufferView((byte*)nullptr, size), layout) {}

	VertexBuffer::VertexBuffer(BufferView vertices, const VertexLayout& layout)
		: m_Layout(layout)
	{
		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.Size, vertices.Data, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	void VertexBuffer::SetData(BufferView vertices) {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.Size, vertices.Data, GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::Bind() {
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void VertexBuffer::Unbind() {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}