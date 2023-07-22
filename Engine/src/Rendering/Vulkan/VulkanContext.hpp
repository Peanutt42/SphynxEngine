#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Window.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanShader.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanTexture.hpp"

namespace Sphynx::Rendering {
	class VulkanContext {
	public:
		VulkanContext(Window& window);
		~VulkanContext();

		void Begin();
		void End();

		void WaitBeforeClose();

		void SetFramebufferResized() { m_FramebufferResized = true; }

		Window& GetWindow() { return m_Window; }
		GLFWwindow* GetWindowHandle() { return m_Window.GetGLFWHandle(); }

		VkInstance GetVkInstance() { return m_Instance->Instance; }
		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
		VkDevice GetDevice() { return m_LogicalDevice; }
		VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
		uint32_t GetMaxFramesInFlight() { return m_MaxFramesInFlight; }
		VulkanCommandPool& GetCommandPool() { return *m_CommandPool; }
		VulkanRenderpass& GetRenderpass() { return *m_Renderpass; }
		VkCommandBuffer GetCurrentCommandBuffer() { return m_CommandBuffer; }

		VkDescriptorPool GetImGuiDescriptorPool();

	private:
		void _CreateSyncObjects(), _DestroySyncObjects();

	private:
		Window& m_Window;

		std::unique_ptr<VulkanInstance> m_Instance;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_LogicalDevice = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;
		std::unique_ptr<VulkanSwapChain> m_SwapChain;
		std::unique_ptr<VulkanRenderpass> m_Renderpass;
		std::unique_ptr<VulkanCommandPool> m_CommandPool;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_RenderFinishedSemaphores;
		std::vector<VkFence> m_InFlightFences;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;

		std::unique_ptr<VulkanRenderpass> m_SceneRenderpass;
		uint32_t m_SceneWidth = 1920, m_SceneHeight = 1080;

		VkSharingMode m_SharingMode = VK_SHARING_MODE_MAX_ENUM;

		uint32_t m_MaxFramesInFlight = 2;
		uint32_t m_CurrentFrame = 0;
		uint32_t m_CurrentImage = 0;
		bool m_FramebufferResized = false;

		VkDescriptorPool m_ImGuiDescriptorPool = VK_NULL_HANDLE;
	};
}