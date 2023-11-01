#pragma once

#include "pch.hpp"
#include <vulkan/vulkan.hpp>

namespace Sphynx::Rendering {
	class SE_API VulkanPipelineCache {
	public:
		VulkanPipelineCache();
		VulkanPipelineCache(const std::filesystem::path& cacheFilepath);
		~VulkanPipelineCache();

		VulkanPipelineCache(const VulkanPipelineCache&) = delete;
		VulkanPipelineCache(VulkanPipelineCache&&) = delete;
		VulkanPipelineCache& operator=(const VulkanPipelineCache&) = delete;
		VulkanPipelineCache& operator=(VulkanPipelineCache&&) = delete;

		bool SaveToFile(const std::filesystem::path& filepath);

		vk::PipelineCache GetHandle() { return m_Cache; }

	private:
		vk::PipelineCache m_Cache;
	};
}