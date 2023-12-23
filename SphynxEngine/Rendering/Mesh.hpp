#pragma once

#include "pch.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Sphynx::Rendering {
	class SE_API Mesh {
	public:
		Mesh(BufferView vertices, const VertexLayout& vertexLayout, const std::vector<uint32>& indices, const VertexLayout& instanceLayout);

		template<typename T>
		Mesh(const std::vector<T>& vertices, const std::vector<uint32>& indices, const VertexLayout& instanceLayout) : Mesh(BufferView(vertices), T::GetVertexLayout(), indices, instanceLayout) {}

		void SetInstances(BufferView instances);

		void Draw();

	private:
		std::shared_ptr<VertexArray> m_VertexArray;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		bool m_Instanced = true;
		std::shared_ptr<VertexBuffer> m_InstanceBuffer;
		std::vector<byte> m_InstanceData;
		int m_InstanceLayoutSize = 0;
		int m_InstanceCount = 0;
	};
}