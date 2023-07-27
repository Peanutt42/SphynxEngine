#include "pch.hpp"
#include "VulkanInstance.hpp"
#include "Core/Engine.hpp"

#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	VulkanInstance::VulkanInstance(bool validation)
		: Validation(validation)
	{
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Sphynx Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Sphynx Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(Engine::Version.Major, Engine::Version.Minor, Engine::Version.Patch);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;


		m_RequiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		_ConfigureExtensions(createInfo);

		if (Validation)
			ValidationLayers.push_back("VK_LAYER_KHRONOS_validation");
		bool wantedValidation = Validation;
		Validation = _ConfigureValidationLayers(createInfo, ValidationLayers);
		if (wantedValidation && !Validation)													// validation layers only work on locally installed VulkanSDK!
			SE_WARN(Logging::Rendering, "Couldn't enable vulkan validation layers.\n    -> Fix: Install VulkanSDK locally from https://vulkan.lunarg.com/sdk/home");

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
		if (Validation) {
			_ConfigureDebugMessengerCreateInfo(debugMessengerCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugMessengerCreateInfo;
		}
		else
			createInfo.pNext = nullptr;


		VkResult result = vkCreateInstance(&createInfo, nullptr, &Instance);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create vulkan instance");

		if (Validation)
			_CreateDebugMessenger(debugMessengerCreateInfo);
	}

	VulkanInstance::~VulkanInstance() {
		if (Validation)
			_DestroyDebugMessenger();

		vkDestroyInstance(Instance, nullptr);
		Instance = VK_NULL_HANDLE;
	}


	void VulkanInstance::_ConfigureExtensions(VkInstanceCreateInfo& createInfo) {
		// Get supported extensions
		uint32_t supportedExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
		m_SupportedExtensionProperties.resize(supportedExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, m_SupportedExtensionProperties.data());

		// Add glfw extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		for (uint32_t i = 0; i < glfwExtensionCount; i++)
			m_RequiredExtensions.emplace_back(glfwExtensions[i]);

		// Check if extensions are supported
		size_t unsupportedExtensionCount = 0;
		for (const char* extension : m_RequiredExtensions) {
			bool foundExtension = false;
			std::string_view extensionStr(extension);
			for (const VkExtensionProperties& extensionProps : m_SupportedExtensionProperties) {
				if (std::string_view(extensionProps.extensionName) == extensionStr) {
					foundExtension = true;
					break;
				}
			}
			if (!foundExtension) {
				SE_ERR(Logging::Rendering, "Unsupported instane extesnion: {}", extension);
				unsupportedExtensionCount++;
			}
		}
		if (unsupportedExtensionCount != 0) {
			SE_FATAL(Logging::Rendering, "{} instance extensions aren't supported! (see logs for more detail)", unsupportedExtensionCount);
		}

		createInfo.enabledExtensionCount = (uint32_t)m_RequiredExtensions.size();
		createInfo.ppEnabledExtensionNames = m_RequiredExtensions.data();
	}


	bool VulkanInstance::_ConfigureValidationLayers(VkInstanceCreateInfo& createInfo, const std::vector<const char*>& validationLayers) {
		// Get supported validation layers
		uint32_t supportedLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&supportedLayerCount, nullptr);
		std::vector<VkLayerProperties> supportedLayers(supportedLayerCount);
		vkEnumerateInstanceLayerProperties(&supportedLayerCount, supportedLayers.data());

		size_t unsupportedValidationLayerCount = 0;
		for (const char* layer : validationLayers) {
			bool foundLayer = false;
			std::string_view layerStr = layer;
			for (const VkLayerProperties& supportedLayerProp : supportedLayers) {
				if (std::string_view(supportedLayerProp.layerName) == layerStr) {
					foundLayer = true;
					break;
				}
			}
			if (!foundLayer) {
				SE_WARN(Logging::Rendering, "{} validation layer isn't supported!", layer);
				unsupportedValidationLayerCount++;
			}
		}
		if (unsupportedValidationLayerCount != 0)
			return false;

		if (Validation) {
			createInfo.enabledLayerCount = (uint32_t)validationLayers.size();
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* /*pUserData*/)
	{

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT &&
			messageType != VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		{
			SE_ERR(Logging::Rendering, "[Validation Layer]: {}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}
	void VulkanInstance::_ConfigureDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	void VulkanInstance::_CreateDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		VkResult result = CreateDebugUtilsMessengerEXT(Instance, &createInfo, nullptr, &m_DebugMessenger);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create debug messenger");
	}

	void VulkanInstance::_DestroyDebugMessenger() {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func) {
			func(Instance, m_DebugMessenger, nullptr);
			m_DebugMessenger = VK_NULL_HANDLE;
		}
	}
}