#include "pch.hpp"
#include "VertexBuffer.hpp"
#include "Profiling/Profiling.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	VertexBuffer::VertexBuffer(uint32 size, const VertexLayout& layout) : VertexBuffer(BufferView((byte*)nullptr, size), layout) {}

	VertexBuffer::VertexBuffer(BufferView vertices, const VertexLayout& layout)
		: m_Layout(layout)
	{
		SE_PROFILE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.Size, vertices.Data, GL_STATIC_DRAW);
	}

	VertexBuffer::~VertexBuffer() {
		SE_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
	}

	void VertexBuffer::SetData(BufferView vertices) {
		SE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ARRAY_BUFFER, vertices.Size, vertices.Data, GL_DYNAMIC_DRAW);
	}

	void VertexBuffer::Bind() {
		SE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	}

	void VertexBuffer::Unbind() {
		SE_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}