#include "pch.hpp"
#include "VertexArray.hpp"
#include "Profiling/Profiling.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	int GetVertexAttribElementType(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int: return GL_INT;
		case VertexAttrib::Float:
		case VertexAttrib::Vec2:
		case VertexAttrib::Vec3:
		case VertexAttrib::Vec4: return GL_FLOAT;
		}
	}

	int GetVertexAttribElementCount(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int:
		case VertexAttrib::Float: return 1;
		case VertexAttrib::Vec2: return 2;
		case VertexAttrib::Vec3: return 3;
		case VertexAttrib::Vec4: return 4;
		}
	}

	size_t GetVertexAttribSize(VertexAttrib attrib) {
		switch (attrib) {
		default: return 0;
		case VertexAttrib::Int: return sizeof(int);
		case VertexAttrib::Float: return sizeof(float);
		case VertexAttrib::Vec2: return 2 * sizeof(float);
		case VertexAttrib::Vec3: return 3 * sizeof(float);
		case VertexAttrib::Vec4: return 4 * sizeof(float);
		}
	}

	int VertexLayout::GetTotalSize() const {
		int total_size = 0;
		for (const auto& attrib : Attributes)
			total_size += GetVertexAttribSize(attrib);
		return total_size;
	}

	VertexArray::VertexArray() {
		SE_PROFILE_FUNCTION();

		glGenVertexArrays(1, &m_ID);
	}

	VertexArray::~VertexArray() {
		SE_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_ID);
		m_ID = 0;
	}

	void VertexArray::AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) {
		SE_PROFILE_FUNCTION();

		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();

		size_t total_vertex_size = layout.GetTotalSize();

		byte* offset = nullptr;
		for (int i = 0; i < layout.Attributes.size(); i++) {
			const auto& attrib = layout.Attributes[i];

			glVertexAttribPointer(m_VertexAttribLoc, GetVertexAttribElementCount(attrib), GetVertexAttribElementType(attrib), GL_FALSE, total_vertex_size, offset);
			glEnableVertexAttribArray(m_VertexAttribLoc);

			m_VertexAttribLoc++;
			offset += GetVertexAttribSize(attrib);
		}
		
		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) {
		SE_PROFILE_FUNCTION();

		glBindVertexArray(m_ID);
		indexBuffer->Bind();
		m_IndexBuffer = indexBuffer;
	}

	void VertexArray::SetInstanceBuffer(std::shared_ptr<VertexBuffer> instanceBuffer) {
		SE_PROFILE_FUNCTION();

		glBindVertexArray(m_ID);
		instanceBuffer->Bind();

		const auto& layout = instanceBuffer->GetLayout();

		size_t total_vertex_size = layout.GetTotalSize();

		byte* offset = nullptr;
		for (int i = 0; i < layout.Attributes.size(); i++) {
			const auto& attrib = layout.Attributes[i];

			glVertexAttribPointer(m_VertexAttribLoc, GetVertexAttribElementCount(attrib), GetVertexAttribElementType(attrib), GL_FALSE, total_vertex_size, offset);
			glEnableVertexAttribArray(m_VertexAttribLoc);
			glVertexAttribDivisor(m_VertexAttribLoc, 1); // per instance, not per vertex

			m_VertexAttribLoc++;
			offset += GetVertexAttribSize(attrib);
		}
	}

	void VertexArray::Bind() {
		SE_PROFILE_FUNCTION();

		glBindVertexArray(m_ID);
	}

	void VertexArray::Unbind() {
		SE_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}
}