#pragma once

#include "pch.hpp"
#include "Rendering/Window.hpp"
#include "VulkanInstance.hpp"
#include "VulkanSwapChain.hpp"
#include "VulkanShader.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanCommandPool.hpp"
#include "VulkanTexture.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanInstanceBuffer.hpp"
#include "Rendering/InstanceData.hpp"

namespace Sphynx::Rendering {
	struct UniformBufferData {
		glm::mat4 proj_view; // proj * view
	};

	class VulkanContext {
	public:
		static void Init(Window& window);
		static void Shutdown();

		static void Begin();
		static void BeginSceneRenderpass();
		static void EndSceneRenderpass();
		static void BeginLastRenderpass();
		static void EndLastRenderpass();
		static void Finish();

		static void WaitBeforeClose();

		static void GenerateSceneTextureDescriptorSets();


		inline static Rendering::Window* Window = nullptr;

		inline static std::unique_ptr<VulkanInstance> Instance;
		inline static vk::SurfaceKHR Surface;
		inline static vk::PhysicalDevice PhysicalDevice;
		inline static vk::Device LogicalDevice;
		inline static vk::Queue GraphicsQueue;
		inline static vk::Queue PresentQueue;
		inline static VulkanSwapChain* SwapChain = nullptr;
		inline static VulkanRenderpass* Renderpass = nullptr;
		inline static VulkanCommandPool* CommandPool = nullptr;

		inline static std::vector<vk::Semaphore> ImageAvailableSemaphores;
		inline static std::vector<vk::Semaphore> RenderFinishedSemaphores;
		inline static std::vector<vk::Fence> InFlightFences;

		inline static vk::CommandBuffer CommandBuffer;

		inline static VulkanRenderpass* SceneRenderpass = nullptr;
		inline static uint32 SceneWidth = 1920;
		inline static uint32 SceneHeight = 1080;
		inline static std::vector<vk::DescriptorSet> SceneTextureDescriptorSets;

		inline static VulkanInstanceBuffer<InstanceData>* InstanceBuffer = nullptr;

		inline static vk::Sampler DefaultSampler;

		inline static vk::SharingMode SharingMode;

		inline static VulkanUniformBuffer* UniformBuffer = nullptr;

		inline static uint32 MaxFramesInFlight = 2;
		inline static uint32 CurrentFrame = 0;
		inline static uint32 CurrentImage = 0;
		inline static bool FramebufferResized = false;

		inline static vk::DescriptorPool DescriptorPool;

	private:
		static void _CreateSyncObjects();
		static void _DestroySyncObjects();
	};
}