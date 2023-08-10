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

		static void BeginSceneRenderpass();
		static void EndSceneRenderpass();
		static void BeginLastRenderpass();
		static void EndLastRenderpass();
		static void Finish();

		static void WaitBeforeClose();


		inline static Rendering::Window* Window = nullptr;

		inline static std::unique_ptr<VulkanInstance> Instance;
		inline static vk::SurfaceKHR Surface;
		inline static vk::PhysicalDevice PhysicalDevice;
		inline static vk::Device LogicalDevice;
		inline static vk::Queue GraphicsQueue;
		inline static vk::Queue PresentQueue;
		inline static std::unique_ptr<VulkanSwapChain> SwapChain;
		inline static std::unique_ptr<VulkanRenderpass> Renderpass;
		inline static std::unique_ptr<VulkanCommandPool> CommandPool;

		inline static std::vector<vk::Semaphore> ImageAvailableSemaphores;
		inline static std::vector<vk::Semaphore> RenderFinishedSemaphores;
		inline static std::vector<vk::Fence> InFlightFences;

		inline static vk::CommandBuffer CommandBuffer;

		inline static std::unique_ptr<VulkanRenderpass> SceneRenderpass;
		inline static uint32_t SceneWidth = 1920;
		inline static uint32_t SceneHeight = 1080;

		inline static vk::SharingMode SharingMode;

		inline static uint32_t MaxFramesInFlight = 2;
		inline static uint32_t CurrentFrame = 0;
		inline static uint32_t CurrentImage = 0;
		inline static bool FramebufferResized = false;

		inline static vk::DescriptorPool ImGuiDescriptorPool;

	private:
		static void _CreateSyncObjects();
		static void _DestroySyncObjects();
	};
}