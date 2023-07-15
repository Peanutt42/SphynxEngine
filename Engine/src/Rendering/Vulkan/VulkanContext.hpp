#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Window.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanShader.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanCommandPool.hpp"

namespace Sphynx::Rendering {
	class VulkanContext {
	public:
		VulkanContext(Window& window);
		~VulkanContext();

		void Update();

		void WaitBeforeClose();

		void SetFramebufferResized() { m_FramebufferResized = true; }

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

		uint32_t m_MaxFramesInFlight = 2;
		uint32_t m_CurrentFrame = 0;
		bool m_FramebufferResized = false;

		std::unique_ptr<VulkanShader> m_TriangleShader;
	};
}