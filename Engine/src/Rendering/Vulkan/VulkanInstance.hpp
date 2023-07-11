#pragma once

#include "Core/CoreInclude.hpp"

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
		void _ConfigureValidationLayers(VkInstanceCreateInfo& createInfo, const std::vector<const char*>& validationLayers);

		void _ConfigureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void _CreateDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& createInfo), _DestroyDebugMessenger();

	private:

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

		std::vector<VkExtensionProperties> m_SupportedExtensionProperties;
		std::vector<const char*> m_RequiredExtensions;
	};
}