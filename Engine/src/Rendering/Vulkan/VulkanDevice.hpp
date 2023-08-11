#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	struct VulkanQueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		VulkanQueueFamilyIndices(VkPhysicalDevice device);

		bool IsComplete() const {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	class VulkanPhysicalDevice {
	public:
		static bool IsDeviceSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, std::vector<const char*>& outUnsupportedExtensions);

		static VkPhysicalDevice Pick(const std::vector<const char*>& deviceExtensions);

		static std::string GetName(VkPhysicalDevice device);
	};

	class VulkanLogicalDevice {
	public:
		struct CreateResult {
			VkDevice Device = VK_NULL_HANDLE;
			VkQueue GraphicsQueue = VK_NULL_HANDLE;
			VkQueue PresentQueue = VK_NULL_HANDLE;
		};

		static CreateResult Create(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions);
	};
}