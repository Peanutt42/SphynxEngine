#include "pch.hpp"
#include "Mesh.hpp"
#include "Vulkan/VulkanShader.hpp"
#include "Vulkan/VulkanContext.hpp"

namespace Sphynx::Rendering {
	VertexInput Vertex::GetInputDescription() {
		VertexInput input;
		input.Description.binding = 0;
		input.Description.stride = sizeof(Vertex);
		input.Description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VulkanVertexAttributeBuilder builder;
		builder.Add(AttributeFormat::Float3, offsetof(Vertex, Position));
		builder.Add(AttributeFormat::Float3, offsetof(Vertex, Normal));
		builder.Add(AttributeFormat::Float2, offsetof(Vertex, UV));
		// Note: set the binding to 1 for per instance data

		input.Attributes = builder.GetAttributes();
		
		return input;
	}
	
	
	void MeshData::LoadMesh(const std::filesystem::path& filepath) {
		SE_ASSERT(filepath.extension() == L".semesh", Logging::Rendering, "Can't load any other file format than .semesh!");

		FileStreamReader in(filepath);
		in.ReadArray(Vertices);
		in.ReadArray(Indices);
	}

	void MeshData::SaveMesh(const std::filesystem::path& filepath) {
		SE_ASSERT(filepath.extension() == L".semesh", Logging::Rendering, "Can't save any other file format than .semesh!");

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
			m_IndexBuffer.reset();
		m_VertexBuffer.reset();
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