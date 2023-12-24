#pragma once

#include "pch.hpp"

namespace Sphynx::Rendering {
	struct Vertex {
		glm::vec3 Position{ 0, 0, 0 };
		glm::vec3 Normal{ 0, 0, 0 };
		glm::vec2 UV{ 0, 0 };
	};

	struct SE_API MeshData {
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;

		// .semesh format only
		void LoadMesh(const std::filesystem::path& filepath);
		// .semesh format only
		void SaveMesh(const std::filesystem::path& filepath);
	};

	class VulkanBuffer;

	class SE_API Mesh {
	public:
		Mesh(const MeshData& data);
		Mesh(BufferView vertices, uint32 vertexCount, const std::vector<uint32>& indices);
		~Mesh();

		void Draw(uint32 instanceCount);

	private:
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) = delete;

	private:
		std::unique_ptr<VulkanBuffer> m_VertexBuffer;
		std::unique_ptr<VulkanBuffer> m_IndexBuffer;
		uint32 m_VertexCount = 0, m_IndicesCount = 0;
	};
}