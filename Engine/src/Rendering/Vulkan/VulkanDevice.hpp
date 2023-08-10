#pragma once

#include "Core/CoreInclude.hpp"

#include <vulkan/vulkan.hpp>

namespace Sphynx::Rendering {
	struct VulkanQueueFamilyIndices {
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		VulkanQueueFamilyIndices(vk::PhysicalDevice device);

		bool IsComplete() const {
			return GraphicsFamily.has_value() && PresentFamily.has_value();
		}
	};

	class VulkanPhysicalDevice {
	public:
		static bool IsDeviceSupported(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions, std::vector<const char*>& outUnsupportedExtensions);

		static vk::PhysicalDevice Pick(const std::vector<const char*>& deviceExtensions);

		static std::string GetName(vk::PhysicalDevice device);
	};

	class VulkanLogicalDevice {
	public:
		struct CreateResult {
			vk::Device Device;
			vk::Queue GraphicsQueue;
			vk::Queue PresentQueue;
		};

		static CreateResult Create(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions);
	};
}