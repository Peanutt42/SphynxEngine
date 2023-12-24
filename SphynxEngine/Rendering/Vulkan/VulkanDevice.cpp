#include "pch.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	bool VulkanPhysicalDevice::IsDeviceSupported(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions, std::vector<const char*>& outUnsupportedExtensions) {
		vk::PhysicalDeviceFeatures deviceFeatures;
		device.getFeatures(&deviceFeatures);
		if (!deviceFeatures.geometryShader)
			return false;
		if (!deviceFeatures.samplerAnisotropy)
			return false;
		if (!deviceFeatures.fillModeNonSolid)
			return false;
		if (!deviceFeatures.independentBlend)
			return false;

		std::vector<vk::ExtensionProperties> extensions = device.enumerateDeviceExtensionProperties();
		
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

	vk::PhysicalDevice VulkanPhysicalDevice::Pick(const std::vector<const char*>& deviceExtensions) {
		vk::PhysicalDevice physicalDevice;

		std::vector<vk::PhysicalDevice> devices = VulkanContext::Instance->Instance.enumeratePhysicalDevices();

		std::multimap<size_t, vk::PhysicalDevice> gpuChoices;
		std::unordered_map<vk::PhysicalDevice::NativeType, std::vector<const char*>> unsupportedExtensionsMap;
		for (const auto& device : devices) {
			if (!IsDeviceSupported(device, deviceExtensions, unsupportedExtensionsMap[device]))
				continue;

			// give each GPU a score
			size_t score = 0;
			vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
			
			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				score += 1000;
			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;
			
			gpuChoices.insert(std::make_pair(score, device));
		}
		for (const auto&[device, unsupportedExtensions] : unsupportedExtensionsMap) {
			if (unsupportedExtensions.empty())
				continue;

			SE_ERR(Logging::Rendering, "Unsupported extensions for {}:", GetName(device));
			for (const char* extension : unsupportedExtensions)
				SE_ERR(Logging::Rendering, "\t{}", extension);
		}

		SE_ASSERT(!gpuChoices.empty(), Logging::Rendering, "Couldn't find a suitable GPU");

		if (gpuChoices.rbegin()->first > 0)
			physicalDevice = gpuChoices.rbegin()->second;
		else
			SE_FATAL(Logging::Rendering, "Couldn't find a single suitable GPU!");

		return physicalDevice;
	}

	std::string VulkanPhysicalDevice::GetName(vk::PhysicalDevice device) {
		return device.getProperties().deviceName;
	}


	VulkanQueueFamilyIndices::VulkanQueueFamilyIndices(vk::PhysicalDevice device) {
		std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

		for (int i = 0; i < queueFamilies.size(); i++) {
			if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
				GraphicsFamily = i;

			vk::Bool32 presentSupport = false;
			vk::Result result = device.getSurfaceSupportKHR(i, VulkanContext::Surface, &presentSupport);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to get support of a surface");
			if (presentSupport)
				PresentFamily = i;

			if (IsComplete())
				break;
		}
	}


	VulkanLogicalDevice::CreateResult VulkanLogicalDevice::Create(const std::vector<const char*>& validationLayers, const std::vector<const char*>& deviceExtensions) {
		VulkanQueueFamilyIndices indices(VulkanContext::PhysicalDevice);

		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = indices.GraphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		vk::PhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = true;
		deviceFeatures.geometryShader = true;
		deviceFeatures.fillModeNonSolid = true;
		deviceFeatures.independentBlend = true;

		vk::DeviceCreateInfo createInfo{};
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		createInfo.enabledLayerCount = (uint32)validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
		
		createInfo.enabledExtensionCount = (uint32)deviceExtensions.size();
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		std::set<uint32> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		for (uint32 uniqueQueueFamily : uniqueQueueFamilies) {
			vk::DeviceQueueCreateInfo uniqueQueueCreateInfo{};
			uniqueQueueCreateInfo.queueFamilyIndex = uniqueQueueFamily;
			uniqueQueueCreateInfo.queueCount = 1;
			uniqueQueueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		createInfo.queueCreateInfoCount = (uint32)queueCreateInfos.size();
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		CreateResult result;
		vk::Result createResult = VulkanContext::PhysicalDevice.createDevice(&createInfo, nullptr, &result.Device);
		SE_ASSERT(createResult == vk::Result::eSuccess, Logging::Rendering, "Failed to create logical device");

		result.Device.getQueue(indices.GraphicsFamily.value(), 0, &result.GraphicsQueue);
		result.Device.getQueue(indices.PresentFamily.value(), 0, &result.PresentQueue);

		return result;
	}
}