#pragma once

#include "Core/CoreInclude.hpp"

namespace Sphynx::Rendering {
	struct SE_API Vertex {
		glm::vec3 Position{ 0, 0, 0 };
		glm::vec3 Normal{ 0, 0, 0 };
		glm::vec2 UV{ 0, 0 };
	};

	struct SE_API MeshData {
		std::vector<Vertex> Vertices;
		std::vector<uint32_t> Indices;

		// .semesh format only
		void LoadMesh(const std::filesystem::path& filepath);
		// .semesh format only
		void SaveMesh(const std::filesystem::path& filepath);
	};


	class VulkanBuffer;

	class SE_API Mesh {
	public:
		Mesh(const MeshData& data);
		Mesh(BufferView vertices, uint32_t vertexCount, const std::vector<uint32_t>& indices);
		~Mesh();

		void Draw(uint32_t instanceCount);

	private:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

	private:
		VulkanBuffer* m_VertexBuffer = nullptr;
		VulkanBuffer* m_IndexBuffer = nullptr;
		uint32_t m_VertexCount = 0, m_IndicesCount = 0;
	};
}