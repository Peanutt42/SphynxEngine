#include "pch.hpp"
#include "Mesh.hpp"
#include "Serialization/FileStream.hpp"
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
		: Mesh(BufferView(data.Vertices), (uint32)data.Vertices.size(), data.Indices) {}

	Mesh::Mesh(BufferView vertices, uint32 vertexCount, const std::vector<uint32>& indices) {
		m_VertexCount = vertexCount;
		m_VertexBuffer.reset(VulkanBuffer::CreateWithStaging(vertices, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal));
		m_IndicesCount = (uint32)indices.size();
		if (m_IndicesCount > 0)
			m_IndexBuffer.reset(VulkanBuffer::CreateWithStaging(BufferView(indices), vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal));
	}

	Mesh::~Mesh() {
		m_IndexBuffer.reset();
		m_VertexBuffer.reset();
	}

	void Mesh::Draw(uint32 instanceCount) {
		vk::CommandBuffer& cmd = VulkanContext::CommandBuffer;
		
		vk::DeviceSize offset = 0;
		cmd.bindVertexBuffers(0, 1, &m_VertexBuffer->Buffer, &offset);

		if (m_IndicesCount > 0) {
			cmd.bindIndexBuffer(m_IndexBuffer->Buffer, 0, vk::IndexType::eUint32);
			cmd.drawIndexed(m_IndicesCount, instanceCount, 0, 0, 0);
		}
		else {
			cmd.draw(m_VertexCount, instanceCount, 0, 0);
		}
	}
}