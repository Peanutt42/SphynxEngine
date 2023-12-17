#include "pch.hpp"
#include "Mesh.hpp"

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

	Mesh::Mesh(BufferView vertices, const VertexLayout& vertexLayout, const std::vector<uint32>& indices) : m_IndexCount(indices.size()) {
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.Size, vertices.Data, GL_STATIC_DRAW);

		size_t total_vertex_size = 0;
		for (const auto& attrib : vertexLayout.Attributes)
			total_vertex_size += GetVertexAttribSize(attrib);

		byte* offset = nullptr;
		for (int i = 0; i < vertexLayout.Attributes.size(); i++) {
			const auto& attrib = vertexLayout.Attributes[i];

			glVertexAttribPointer(i, GetVertexAttribElementCount(attrib), GetVertexAttribElementType(attrib), GL_FALSE, total_vertex_size, offset);
			glEnableVertexAttribArray(i);

			offset += GetVertexAttribSize(attrib);
		}

		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32), indices.data(), GL_STATIC_DRAW);
	}

	Mesh::~Mesh() {
		glDeleteVertexArrays(1, &m_VAO);
		std::array<uint32, 2> buffers = { m_VBO, m_EBO };
		glDeleteBuffers(buffers.size(), buffers.data());
	}

	void Mesh::Draw() {
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
	}
}