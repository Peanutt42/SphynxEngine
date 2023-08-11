#pragma once

#include "pch.hpp"

#include <vulkan/vulkan.h>

namespace Sphynx::Rendering {
	class VulkanInstance {
	public:
		VkInstance Instance = VK_NULL_HANDLE;

		bool Validation = false;
		std::vector<const char*> ValidationLayers;

		VulkanInstance(bool validation);
		~VulkanInstance();


	private:
		void _ConfigureExtensions(VkInstanceCreateInfo& createInfo);

		// Returns if validation layers are usable (local VulkanSDK installation is required for validation layers!)
		bool _ConfigureValidationLayers(VkInstanceCreateInfo& createInfo, const std::vector<const char*>& validationLayers);

		void _ConfigureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void _CreateDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& createInfo), _DestroyDebugMessenger();

	private:

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		std::vector<VkExtensionProperties> m_SupportedExtensionProperties;
		std::vector<const char*> m_RequiredExtensions;
	};
}