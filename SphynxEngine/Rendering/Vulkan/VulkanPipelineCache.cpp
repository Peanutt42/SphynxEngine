#include "pch.hpp"
#include "VulkanPipelineCache.hpp"
#include "VulkanContext.hpp"
#include "Serialization/MemoryStream.hpp"
#include "Serialization/FileStream.hpp"

namespace Sphynx::Rendering {
	VulkanPipelineCache::VulkanPipelineCache() {
		vk::PipelineCacheCreateInfo createInfo{};
		m_Cache = VulkanContext::LogicalDevice.createPipelineCache(createInfo);
	}

	VulkanPipelineCache::VulkanPipelineCache(const std::filesystem::path& cacheFilepath) {
		vk::PhysicalDeviceProperties deviceProperties = VulkanContext::PhysicalDevice.getProperties();

		vk::PipelineCacheCreateInfo createInfo{};

		std::vector<byte> cacheData;
		bool cacheValid = false;
		if (FileStreamReader::ReadBinaryFile(cacheFilepath, cacheData)) {
			cacheValid = true;

			vk::PipelineCacheHeaderVersionOne header{};
			MemoryStreamReader reader = MemoryStreamReader(BufferView(cacheData));
			reader.Read(header.headerSize);
			if (header.headerSize != sizeof(header)) {
				SE_ERR(Logging::Rendering, "Invalid pipeline cache header, header size should be {} but instead is {}", sizeof(header), header.headerSize);
				cacheValid = false;
			}
			if (cacheData.size() < header.headerSize) {
				SE_ERR(Logging::Rendering, "Pipeline cache specifies a invalid header size!");
				cacheValid = false;
			}
			else {
				reader.Read(header.headerVersion);
				if (header.headerVersion != vk::PipelineCacheHeaderVersion::eOne) {
					SE_ERR(Logging::Rendering, "Unsupported pipeline cache header version (Version ONE expected)!");
					cacheValid = false;
				}
				reader.Read(header.vendorID);
				if (header.vendorID != deviceProperties.vendorID) {
					SE_ERR(Logging::Rendering, "Pipeline cache header vendorID missmatch, cache: {}, driver: {}", header.vendorID, deviceProperties.vendorID);
					cacheValid = false;
				}
				reader.Read(header.deviceID);
				if (header.deviceID != deviceProperties.deviceID) {
					SE_ERR(Logging::Rendering, "Pipeline cache header deviceID missmatch, cache: {}, driver: {}", header.deviceID, deviceProperties.deviceID);
					cacheValid = false;
				}
				reader.Read(header.pipelineCacheUUID);
				if (std::memcmp(header.pipelineCacheUUID, deviceProperties.pipelineCacheUUID, sizeof(deviceProperties.pipelineCacheUUID)) != 0) {
					auto printUUID = [](std::stringstream& ss, const uint8_t uuid[VK_UUID_SIZE]) {
						for (int i = 0; i < VK_UUID_SIZE; ++i) {
							ss << std::setw(2) << (uint32_t)uuid[i];
							if (i == 3 || i == 5 || i == 7 || i == 9)
								ss << '-';
						}
					};
					std::stringstream errorMsg;
					errorMsg << "cache: ";
					printUUID(errorMsg, header.pipelineCacheUUID);
					errorMsg << ", driver: ";
					printUUID(errorMsg, deviceProperties.pipelineCacheUUID);
					SE_ERR(Logging::Rendering, "Pipeline cache UUID missmatch, {}", errorMsg.str());
					cacheValid = false;
				}
			}
		}

		if (cacheValid) {
			createInfo.initialDataSize = cacheData.size();
			createInfo.pInitialData = cacheData.data();
		}
		else {
			try { std::filesystem::remove(cacheFilepath); }
			catch (...) {}
		}

		m_Cache = VulkanContext::LogicalDevice.createPipelineCache(createInfo);
	}

	VulkanPipelineCache::~VulkanPipelineCache() {
		VulkanContext::LogicalDevice.destroyPipelineCache(m_Cache);
		m_Cache = VK_NULL_HANDLE;
	}

	bool VulkanPipelineCache::SaveToFile(const std::filesystem::path& filepath) {
		std::ofstream cacheFile(filepath, std::ios::binary);
		if (!cacheFile.is_open())
			return false;

		std::vector<unsigned char> result = VulkanContext::LogicalDevice.getPipelineCacheData(m_Cache);
		cacheFile.write((const char*)result.data(), result.size() * sizeof(unsigned char));
		return true;
	}
}