#include "pch.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanShader.hpp"
#include "Vulkan/VulkanContext.hpp"
#include "Vulkan/VulkanBuffer.hpp"

namespace Sphynx::Rendering {
	void MeshData::LoadMesh(const std::filesystem::path& filepath) {
		if (filepath.extension() != L".semesh") {
			SE_ERR(Logging::Rendering, "Can't load any other file format than .semesh!");
			return;
		}

		FileStreamReader in(filepath);
		in.ReadArray(Vertices);
		in.ReadArray(Indices);
	}

	void MeshData::SaveMesh(const std::filesystem::path& filepath) {
		if (filepath.extension() != L".semesh") {
			SE_ERR(Logging::Rendering, "Can't save any other file format than .semesh!");
			return;
		}

		FileStreamWriter out(filepath);
		out.WriteArray(Vertices);
		out.WriteArray(Indices);
	}



	Mesh::Mesh(const MeshData& data)
		: Mesh(BufferView(data.Vertices), (uint32_t)data.Vertices.size(), data.Indices) {}

	Mesh::Mesh(BufferView vertices, uint32_t vertexCount, const std::vector<uint32_t>& indices) {
		m_VertexCount = vertexCount;
		m_VertexBuffer = VulkanBuffer::CreateWithStaging(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		m_IndicesCount = (uint32_t)indices.size();
		if (m_IndicesCount > 0)
			m_IndexBuffer = VulkanBuffer::CreateWithStaging(BufferView(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	Mesh::~Mesh() {
		if (m_IndexBuffer)
			delete m_IndexBuffer;
		delete m_VertexBuffer;
	}

	void Mesh::Draw(uint32_t instanceCount) {
		VkCommandBuffer cmd = VulkanContext::CommandBuffer;
		
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &m_VertexBuffer->Buffer, &offset);

		if (m_IndicesCount > 0) {
			vkCmdBindIndexBuffer(cmd, m_IndexBuffer->Buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(cmd, m_IndicesCount, instanceCount, 0, 0, 0);
		}
		else {
			vkCmdDraw(cmd, m_VertexCount, instanceCount, 0, 0);
		}
	}
}