#include "pch.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"

#include "Rendering/InstanceData.hpp"
#include "Profiling/Profiling.hpp"
#include <backends/imgui_impl_vulkan.h>
#include <GLFW/glfw3.h>

namespace Sphynx::Rendering {
	void VulkanContext::Init(Rendering::Window& window) {
		SE_PROFILE_FUNCTION();

		Window = &window;

		Instance = new VulkanInstance(
#if defined(DEBUG) || defined(DEVELOPMENT)
			true
#else
			false
#endif
		);
		
		VkSurfaceKHR tempSurface = nullptr;
		VkResult createSurfaceResult = glfwCreateWindowSurface(Instance->Instance, window.GetGLFWHandle(), nullptr, &tempSurface);
		if (createSurfaceResult == VK_SUCCESS)
			Surface = tempSurface;
		else
			SE_FATAL(Logging::Rendering, "Failed to create window surface");

		std::vector<const char*> deviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		
		PhysicalDevice = VulkanPhysicalDevice::Pick(deviceExtensions);
		
		SE_INFO(Logging::Rendering, "Vulkan SDK {}.{}.{}: {}",
			VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE), VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE),
			VulkanPhysicalDevice::GetName(PhysicalDevice));
		
		VulkanLogicalDevice::CreateResult logicalDeviceResult = VulkanLogicalDevice::Create(Instance->ValidationLayers, deviceExtensions);
		LogicalDevice = logicalDeviceResult.Device;
		GraphicsQueue = logicalDeviceResult.GraphicsQueue;
		PresentQueue = logicalDeviceResult.PresentQueue;
		
		VulkanQueueFamilyIndices indices(PhysicalDevice);
		SharingMode = indices.GraphicsFamily == indices.PresentFamily ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;

		VulkanSwapChain::SupportDetails swapChainSupport = VulkanSwapChain::GetSupport(PhysicalDevice);
		MaxFramesInFlight = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && MaxFramesInFlight > swapChainSupport.Capabilities.maxImageCount)
			MaxFramesInFlight = swapChainSupport.Capabilities.maxImageCount;


		SwapChain = new VulkanSwapChain();

		SceneRenderpass = new VulkanRenderpass(RenderPassUsage::First, SwapChain->GetFormat());

		Renderpass = new VulkanRenderpass(RenderPassUsage::Last, SwapChain->GetFormat());

		SwapChain->CreateFramebuffers(Renderpass->GetHandle());

		CommandPool = new VulkanCommandPool();

		_CreateSyncObjects();

		SceneRenderpass->CreateFramebuffers(SceneWidth, SceneHeight, SwapChain->GetFormat(), true);

		DefaultSampler = VulkanTexture::CreateSampler();

		PipelineCacheFilepath = Engine::GetProject().CacheFolder / "ShaderPipeline.cache";
		PipelineCache = new VulkanPipelineCache(PipelineCacheFilepath);

		UniformBuffer = new VulkanUniformBuffer(sizeof(UniformBufferData));

		// TODO: Change tutorial sizes to more acurate ones
		// NOTE: you can change these values at runtime
		std::array<vk::DescriptorPoolSize, 11> poolSizes = {
			vk::DescriptorPoolSize{ vk::DescriptorType::eSampler, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eSampledImage, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eStorageImage, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBuffer, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eStorageBuffer, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			vk::DescriptorPoolSize{ vk::DescriptorType::eInputAttachment, 1000 }
		};
		vk::DescriptorPoolCreateInfo pool_info{};
		pool_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();
		vk::Result result = LogicalDevice.createDescriptorPool(&pool_info, nullptr, &DescriptorPool);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create descriptor pool");


		InstanceBuffer = new VulkanInstanceBuffer(sizeof(InstanceData));
	}

	void VulkanContext::Shutdown() {
		SE_PROFILE_FUNCTION();

		if (!PipelineCache->SaveToFile(PipelineCacheFilepath))
			SE_WARN(Logging::Rendering, "Failed to save pipeline cache to '{}'", PipelineCacheFilepath.string());

		delete InstanceBuffer;

		delete PipelineCache;

		LogicalDevice.destroySampler(DefaultSampler);

        delete UniformBuffer;

		_DestroySyncObjects();

        delete CommandPool;

        delete Renderpass;

        delete SceneRenderpass;

        delete SwapChain;

		LogicalDevice.destroyDescriptorPool(DescriptorPool, nullptr);

		LogicalDevice.destroy(nullptr);
		LogicalDevice = VK_NULL_HANDLE;

		Instance->Instance.destroySurfaceKHR(Surface, nullptr);
		Surface = VK_NULL_HANDLE;

		delete Instance;
	}

	void VulkanContext::Begin() {
		SE_PROFILE_FUNCTION();

		vk::Result result = LogicalDevice.waitForFences(1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to wait for inFlightFence");

		result = LogicalDevice.acquireNextImageKHR(SwapChain->GetHandle(), UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &CurrentImage);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			SwapChain->Recreate(Renderpass->GetHandle());
			return;
		}
		else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		SE_FATAL(Logging::Rendering, "Failed to acquire swapchain image!");

		// only reset if we are submitting work
		result = LogicalDevice.resetFences(1, &InFlightFences[CurrentFrame]);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to reset inFlightFence");

		CommandBuffer = CommandPool->BeginRecording(CurrentFrame);
	}

	void VulkanContext::BeginSceneRenderpass() {
		SE_PROFILE_FUNCTION();

		SceneRenderpass->Begin(SceneRenderpass->GetFramebuffer(CurrentImage), CommandBuffer, vk::Extent2D(SceneWidth, SceneHeight));
	}

	void VulkanContext::EndSceneRenderpass() {
		SE_PROFILE_FUNCTION();

		SceneRenderpass->End(CommandBuffer);

		// Make Scene Texture readable for shaders
		{
			vk::ImageMemoryBarrier barrier;
			barrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = SceneRenderpass->GetImage(CurrentImage);
			barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
			CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eFragmentShader, (vk::DependencyFlags)0, 0, nullptr, 0, nullptr, 1, &barrier);
		}
	}

	void VulkanContext::BeginLastRenderpass() {
		SE_PROFILE_FUNCTION();

		Renderpass->Begin(SwapChain->GetFramebuffer(CurrentImage), CommandBuffer, SwapChain->GetExtent());
	}

	void VulkanContext::EndLastRenderpass() {
		SE_PROFILE_FUNCTION();

		Renderpass->End(CommandBuffer);

		CommandPool->EndRecording(CurrentFrame);
	}

	void VulkanContext::Finish() {
		// Submit
		{
			SE_PROFILE_SCOPE("Submit");

			vk::SubmitInfo submitInfo{};
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &ImageAvailableSemaphores[CurrentFrame];
			vk::PipelineStageFlags waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			submitInfo.pWaitDstStageMask = &waitDstStageMask;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &RenderFinishedSemaphores[CurrentFrame];
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &CommandBuffer;

			vk::Result result = GraphicsQueue.submit(1, &submitInfo, InFlightFences[CurrentFrame]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to submit commandBuffer");

			CommandBuffer = VK_NULL_HANDLE;
		}

		// Present
		{
			SE_PROFILE_SCOPE("Present");

			vk::PresentInfoKHR presentInfo{};
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentFrame];

			presentInfo.swapchainCount = 1;
			vk::SwapchainKHR swapchain = SwapChain->GetHandle();
			presentInfo.pSwapchains = &swapchain;
			presentInfo.pImageIndices = &CurrentImage;
			presentInfo.pResults = nullptr;

			vk::Result result = PresentQueue.presentKHR(&presentInfo);
			if (result == vk::Result::eErrorOutOfDateKHR ||
				result == vk::Result::eSuboptimalKHR)
			{
				SwapChain->Recreate(Renderpass->GetHandle());
			}
			else if (result != vk::Result::eSuccess)
				SE_FATAL(Logging::Rendering, "Failed to present swapchain image!");
		}

		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
	}

	void VulkanContext::WaitBeforeClose() {
		LogicalDevice.waitIdle();
	}

	void VulkanContext::GenerateSceneTextureDescriptorSets() {
		SceneTextureDescriptorSets.resize(MaxFramesInFlight);
		for (size_t i = 0; i < SceneTextureDescriptorSets.size(); i++)
			SceneTextureDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(DefaultSampler, SceneRenderpass->GetImageView((uint32)i), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void VulkanContext::_CreateSyncObjects() {
		ImageAvailableSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
		RenderFinishedSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
		InFlightFences.resize(MaxFramesInFlight, VK_NULL_HANDLE);

		for (uint32 frameIndex = 0; frameIndex < MaxFramesInFlight; frameIndex++) {
			vk::SemaphoreCreateInfo semaphoreInfo{};
			vk::Result result = LogicalDevice.createSemaphore(&semaphoreInfo, nullptr, &ImageAvailableSemaphores[frameIndex]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create semaphore");
			result = LogicalDevice.createSemaphore(&semaphoreInfo, nullptr, &RenderFinishedSemaphores[frameIndex]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create semaphore");

			vk::FenceCreateInfo fenceInfo{};
			fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
			result = LogicalDevice.createFence(&fenceInfo, nullptr, &InFlightFences[frameIndex]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create inFlightFence");
		}
	}

	void VulkanContext::_DestroySyncObjects() {
		for (uint32 frameIndex = 0; frameIndex < MaxFramesInFlight; frameIndex++) {
			LogicalDevice.destroySemaphore(ImageAvailableSemaphores[frameIndex], nullptr);
			LogicalDevice.destroySemaphore(RenderFinishedSemaphores[frameIndex], nullptr);
			LogicalDevice.destroyFence(InFlightFences[frameIndex], nullptr);
		}
	}
}