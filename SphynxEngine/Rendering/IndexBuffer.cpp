#include "pch.hpp"
#include "IndexBuffer.hpp"
#include "Profiling/Profiling.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	IndexBuffer::IndexBuffer(const std::vector<uint32>& indices) : m_IndexCount(indices.size()) {
		SE_PROFILE_FUNCTION();

		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32), indices.data(), GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer() {
		SE_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_ID);
	}

	void IndexBuffer::Bind() {
		SE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
	}

	void IndexBuffer::Unbind() {
		SE_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}