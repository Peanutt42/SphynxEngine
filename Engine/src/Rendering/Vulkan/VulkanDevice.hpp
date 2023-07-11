#pragma once

#include "Core/CoreInclude.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	struct VulkanQueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		VulkanQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

		bool IsComplete() const {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	class VulkanPhysicalDevice {
	public:
		static bool IsDeviceSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, VkSurfaceKHR surface, std::vector<const char*>& outUnsupportedExtensions);

		static VkPhysicalDevice Pick(VkInstance instance, VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
	};

	class VulkanLogicalDevice {
	public:
		struct CreateResult {
			VkDevice Device = VK_NULL_HANDLE;
			VkQueue GraphicsQueue = VK_NULL_HANDLE;
			VkQueue PresentQueue = VK_NULL_HANDLE;
		};

		static CreateResult Create(VkPhysicalDevice physicalDevice, const std::vector<const char*>& validationLayers, VkSurfaceKHR surface);
	};
}