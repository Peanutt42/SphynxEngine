#include "pch.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSurface.hpp"

namespace Sphynx::Rendering {
	void VulkanContext::Init(Rendering::Window& window) {
		SE_PROFILE_FUNCTION();

		Window = &window;

		Instance = std::make_unique<VulkanInstance>(
#if defined(DEBUG) || defined(RELEASE)
			true
#else
			false
#endif
		);

		Surface = VulkanSurface::GetFromWindow(Instance->Instance, window.GetGLFWHandle());

		std::vector<const char*> deviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		PhysicalDevice = VulkanPhysicalDevice::Pick(deviceExtensions);
		
		SE_INFO(Logging::Rendering, "Chosen GPU: {}", VulkanPhysicalDevice::GetName(PhysicalDevice));
		
		VulkanQueueFamilyIndices indices(PhysicalDevice);
		SharingMode = indices.GraphicsFamily == indices.PresentFamily ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;

		VulkanSwapChain::SupportDetails swapChainSupport = VulkanSwapChain::GetSupport(PhysicalDevice);
		MaxFramesInFlight = swapChainSupport.Capabilities.minImageCount + 1;
		if (swapChainSupport.Capabilities.maxImageCount > 0 && MaxFramesInFlight > swapChainSupport.Capabilities.maxImageCount)
			MaxFramesInFlight = swapChainSupport.Capabilities.maxImageCount;

		VulkanLogicalDevice::CreateResult logicalDeviceResult = VulkanLogicalDevice::Create(Instance->ValidationLayers, deviceExtensions);
		LogicalDevice = logicalDeviceResult.Device;
		GraphicsQueue = logicalDeviceResult.GraphicsQueue;
		GraphicsQueue = logicalDeviceResult.PresentQueue;
		PresentQueue = logicalDeviceResult.PresentQueue;

		SwapChain = std::make_unique<VulkanSwapChain>();

		SceneRenderpass = std::make_unique<VulkanRenderpass>(RenderPassUsage::First, SwapChain->GetFormat());

		Renderpass = std::make_unique<VulkanRenderpass>(RenderPassUsage::Last, SwapChain->GetFormat());

		SwapChain->CreateFramebuffers(Renderpass->GetHandle());

		CommandPool = std::make_unique<VulkanCommandPool>();

		_CreateSyncObjects();

		SceneRenderpass->CreateFramebuffers(SceneWidth, SceneHeight, SwapChain->GetFormat());

		// ImGui DescriptorPool
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
		pool_info.poolSizeCount = (uint32_t)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();
		vk::Result result = LogicalDevice.createDescriptorPool(&pool_info, nullptr, &ImGuiDescriptorPool);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create imgui descriptor pool");
	}

	void VulkanContext::Shutdown() {
		SE_PROFILE_FUNCTION();

		_DestroySyncObjects();

		CommandPool.reset();

		Renderpass.reset();

		SceneRenderpass.reset();

		SwapChain.reset();

		if (ImGuiDescriptorPool)
			LogicalDevice.destroyDescriptorPool(ImGuiDescriptorPool, nullptr);

		LogicalDevice.destroy(nullptr);
		LogicalDevice = VK_NULL_HANDLE;

		Instance->Instance.destroySurfaceKHR(Surface, nullptr);
		Surface = VK_NULL_HANDLE;

		Instance.reset();
	}

	void VulkanContext::BeginSceneRenderpass() {
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

		SceneRenderpass->Begin(SceneRenderpass->GetFramebuffer(CurrentImage), CommandBuffer, VkExtent2D(SceneWidth, SceneHeight));
	}

	void VulkanContext::EndSceneRenderpass() {
		SE_PROFILE_FUNCTION();

		SceneRenderpass->End(CommandBuffer);

		//// Make Scene Texture readable for shaders
		//{
		//	VkImageMemoryBarrier barrier{};
		//	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		//	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//	barrier.image = m_SceneImages[m_CurrentImage];
		//	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//	barrier.subresourceRange.baseMipLevel = 0;
		//	barrier.subresourceRange.levelCount = 1;
		//	barrier.subresourceRange.baseArrayLayer = 0;
		//	barrier.subresourceRange.layerCount = 1;
		//	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		//	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		//	vkCmdPipelineBarrier(m_CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		//}
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
				result == vk::Result::eSuboptimalKHR ||
				FramebufferResized)
			{
				FramebufferResized = false;
				SwapChain->Recreate(Renderpass->GetHandle());
			}
			else if (result != vk::Result::eSuccess)
				SE_FATAL(Logging::Rendering, "Failed to present swapchain image!");
		}

		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
	}

	void VulkanContext::WaitBeforeClose() {
		vkDeviceWaitIdle(LogicalDevice);
	}

	void VulkanContext::_CreateSyncObjects() {
		ImageAvailableSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
		RenderFinishedSemaphores.resize(MaxFramesInFlight, VK_NULL_HANDLE);
		InFlightFences.resize(MaxFramesInFlight, VK_NULL_HANDLE);

		for (uint32_t frameIndex = 0; frameIndex < MaxFramesInFlight; frameIndex++) {
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
		for (uint32_t frameIndex = 0; frameIndex < MaxFramesInFlight; frameIndex++) {
			LogicalDevice.destroySemaphore(ImageAvailableSemaphores[frameIndex], nullptr);
			LogicalDevice.destroySemaphore(RenderFinishedSemaphores[frameIndex], nullptr);
			LogicalDevice.destroyFence(InFlightFences[frameIndex], nullptr);
		}
	}
}