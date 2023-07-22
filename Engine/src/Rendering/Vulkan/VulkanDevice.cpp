#include "pch.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	bool VulkanPhysicalDevice::IsDeviceSupported(VkPhysicalDevice device, const std::vector<const char*>& deviceExtensions, std::vector<const char*>& outUnsupportedExtensions) {
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		if (!deviceFeatures.geometryShader)
			return false;
		if (!deviceFeatures.samplerAnisotropy)
			return false;
		if (!deviceFeatures.fillModeNonSolid)
			return false;
		if (!deviceFeatures.independentBlend)
			return false;

		// Extentions
		uint32_t extensionCount = 0;
		std::vector<VkExtensionProperties> extensions;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		extensions.resize(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

		for (const char* extension : deviceExtensions) {
			bool foundExtension = false;
			std::string_view extensionStr = extension;
			for (auto& ext : extensions) {
				if (std::string_view(ext.extensionName) == extensionStr) {
					foundExtension = true;
					break;
				}
			}
			if (!foundExtension)
				outUnsupportedExtensions.push_back(extension);
		}
		if (!outUnsupportedExtensions.empty())
			return false;

		VulkanSwapChain::SupportDetails swapChainSupport = VulkanSwapChain::GetSupport(device);
		if (swapChainSupport.Formats.empty() || swapChainSupport.PresentModes.empty())
			return false;

		VulkanQueueFamilyIndices indices(device);
		return indices.IsComplete();
	}

	VkPhysicalDevice VulkanPhysicalDevice::Pick(const std::vector<const char*>& deviceExtensions) {
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(VulkanContext::Instance->Instance, &deviceCount, nullptr);
		SE_ASSERT(deviceCount > 0, Logging::Rendering, "Couldn't find a single GPU with Vulkan support!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(VulkanContext::Instance->Instance, &deviceCount, devices.data());

		std::multimap<int, VkPhysicalDevice> gpuChoices;
		std::unordered_map<VkPhysicalDevice, std::vector<const char*>> unsupportedExtensionsMap;
		for (const auto& device : devices) {
			if (!IsDeviceSupported(device, deviceExtensions, unsupportedExtensionsMap[device]))
				continue;

			// give each GPU a score
			int score = 0;
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				score += 1000;
			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;
			
			gpuChoices.insert(std::make_pair(score, device));
		}
		for (const auto&[device, unsupportedExtensions] : unsupportedExtensionsMap) {
			if (unsupportedExtensions.empty())
				continue;

			SE_ERR(Logging::Rendering, "Unsupported extensions for {}:", VulkanPhysicalDevice::GetName(device));
			for (const char* extension : unsupportedExtensions)
				SE_ERR(Logging::Rendering, "\t{}", extension);
		}
		
		SE_ASSERT(gpuChoices.size() != 0, Logging::Rendering, "Couldn't find a suitable GPU");

		if (gpuChoices.rbegin()->first > 0)
			physicalDevice = gpuChoices.rbegin()->second;
		else
			SE_FATAL(Logging::Rendering, "Couldn't find a single suitable GPU!");

		return physicalDevice;
	}

	std::string VulkanPhysicalDevice::GetName(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		return deviceProperties.deviceName;
	}


	VulkanQueueFamilyIndices::VulkanQueueFamilyIndices(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		for (int i = 0; i < queueFamilies.size(); i++) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				GraphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, VulkanContext::Surface, &presentSupport);
			if (presentSupport)
				PresentFamily = i;

			if (IsComplete())
				break;
		}
	}


	VulkanLogicalDevice::CreateResult VulkanLogicalDevice::Create(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions) {
		VulkanQueueFamilyIndices indices(VulkanContext::PhysicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.GraphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = true;
		deviceFeatures.geometryShader = true;
		deviceFeatures.fillModeNonSolid = true;
		deviceFeatures.independentBlend = true;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		std::set<uint32_t> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (uint32_t uniqueQueueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo uniqueQueueCreateInfo{};
			uniqueQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			uniqueQueueCreateInfo.queueFamilyIndex = uniqueQueueFamily;
			uniqueQueueCreateInfo.queueCount = 1;
			uniqueQueueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		CreateResult result;
		if (vkCreateDevice(VulkanContext::PhysicalDevice, &createInfo, nullptr, &result.Device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(result.Device, indices.GraphicsFamily.value(), 0, &result.GraphicsQueue);
		vkGetDeviceQueue(result.Device, indices.PresentFamily.value(), 0, &result.PresentQueue);

		return result;
	}
}