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
	class SE_API VulkanContext {
	public:
		static void Init(Window& window);
		static void Shutdown();

		static void Begin();
		static void End();

		static void WaitBeforeClose();


		inline static Rendering::Window* Window = nullptr;

		inline static std::unique_ptr<VulkanInstance> Instance;
		inline static VkSurfaceKHR Surface = VK_NULL_HANDLE;
		inline static VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		inline static VkDevice LogicalDevice = VK_NULL_HANDLE;
		inline static VkQueue GraphicsQueue = VK_NULL_HANDLE;
		inline static VkQueue PresentQueue = VK_NULL_HANDLE;
		inline static std::unique_ptr<VulkanSwapChain> SwapChain;
		inline static std::unique_ptr<VulkanRenderpass> Renderpass;
		inline static std::unique_ptr<VulkanCommandPool> CommandPool;

		inline static std::vector<VkSemaphore> ImageAvailableSemaphores;
		inline static std::vector<VkSemaphore> RenderFinishedSemaphores;
		inline static std::vector<VkFence> InFlightFences;

		inline static VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

		inline static std::unique_ptr<VulkanRenderpass> SceneRenderpass;
		inline static uint32_t SceneWidth = 1920;
		inline static uint32_t SceneHeight = 1080;

		inline static VkSharingMode SharingMode = VK_SHARING_MODE_MAX_ENUM;

		inline static uint32_t MaxFramesInFlight = 2;
		inline static uint32_t CurrentFrame = 0;
		inline static uint32_t CurrentImage = 0;
		inline static bool FramebufferResized = false;

		inline static VkDescriptorPool ImGuiDescriptorPool = VK_NULL_HANDLE;

	private:
		static void _CreateSyncObjects();
		static void _DestroySyncObjects();
	};
}