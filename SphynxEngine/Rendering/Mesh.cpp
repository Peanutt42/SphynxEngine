#include "pch.hpp"
#include "Mesh.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	Mesh::Mesh(BufferView vertices, const VertexLayout& vertexLayout, const std::vector<uint32>& indices) {
		m_VertexArray = std::make_shared<VertexArray>();
		m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, vertexLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		m_IndexBuffer = std::make_shared<IndexBuffer>(indices);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
	}

	void Mesh::Draw() {
		m_VertexArray->Bind();
		m_IndexBuffer->Bind();
		glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
}