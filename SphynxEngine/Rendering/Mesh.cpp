#include "pch.hpp"
#include "Mesh.hpp"

#include <glad/glad.h>

namespace Sphynx::Rendering {
	Mesh::Mesh(BufferView vertices, const VertexLayout& vertexLayout, const std::vector<uint32>& indices, const VertexLayout& instanceLayout) : m_Instanced(true) {
		m_VertexArray = std::make_shared<VertexArray>();
		m_VertexBuffer = std::make_shared<VertexBuffer>(vertices, vertexLayout);
		m_VertexArray->AddVertexBuffer(m_VertexBuffer);
		m_InstanceBuffer = std::make_shared<VertexBuffer>(32, instanceLayout);
		m_VertexArray->SetInstanceBuffer(m_InstanceBuffer);
		m_IndexBuffer = std::make_shared<IndexBuffer>(indices);
		m_VertexArray->SetIndexBuffer(m_IndexBuffer);
		m_InstanceLayoutSize = instanceLayout.GetTotalSize();
	}

	void Mesh::SetInstances(BufferView instances) {
		m_InstanceData.resize(instances.Size);
		std::memcpy(m_InstanceData.data(), instances.Data, instances.Size);
		m_InstanceCount = instances.Size / m_InstanceLayoutSize;
	}

	void Mesh::Draw() {
		m_VertexArray->Bind();
		if (m_Instanced) {
			m_InstanceBuffer->SetData(m_InstanceData);
			glDrawElementsInstanced(GL_TRIANGLES, m_IndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0, m_InstanceCount);
			m_InstanceData.resize(0);
		}
		else {
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetIndexCount(), GL_UNSIGNED_INT, 0);
		}
	}
}