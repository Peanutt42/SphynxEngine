#pragma once

#include "pch.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"

namespace Sphynx::Rendering {
	inline static int GetVertexAttribElementType(VertexAttrib attrib);
	inline static int GetVertexAttribElementCount(VertexAttrib attrib);
	inline static size_t GetVertexAttribSize(VertexAttrib attrib);


	class SE_API VertexArray {
	public:
		VertexArray();
		~VertexArray();

		void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer);

		void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer);

		void SetInstanceBuffer(std::shared_ptr<VertexBuffer> instanceBuffer);

		void Bind();
		void Unbind();

	private:
		uint32 m_ID = 0;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
		std::shared_ptr<VertexBuffer> m_InstanceBuffer;
		int m_VertexAttribLoc = 0;
	};
}