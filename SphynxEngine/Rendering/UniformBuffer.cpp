#include "pch.hpp"
#include "UniformBuffer.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	UniformBuffer::UniformBuffer(size_t size)
		: m_Size(size), m_Binding(s_NextBinding++)
	{
		glGenBuffers(1, &m_ID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_ID);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	UniformBuffer::~UniformBuffer() {
		glDeleteBuffers(1, &m_ID);
	}

	void UniformBuffer::Update(BufferView data) {
		if (data.Size != m_Size) {
			SE_WARN(Logging::Rendering, "Invalid UniformBuffer::Update dataSize! Should be {} but is {}", m_Size, data.Size);
			return;
		}
		
		glBindBuffer(GL_UNIFORM_BUFFER, m_ID);
		glBufferData(GL_UNIFORM_BUFFER, m_Size, data.Data, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}