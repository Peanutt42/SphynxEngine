#include "pch.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSurface.hpp"

namespace Sphynx::Rendering {
	void VulkanContext::Init(Rendering::Window& window) {
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
		SharingMode = indices.GraphicsFamily == indices.PresentFamily ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;

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
		std::array<VkDescriptorPoolSize, 11> poolSizes = {
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();
		vkCreateDescriptorPool(LogicalDevice, &pool_info, nullptr, &ImGuiDescriptorPool);
	}

	void VulkanContext::Shutdown() {
		_DestroySyncObjects();

		CommandPool.reset();

		Renderpass.reset();

		SceneRenderpass.reset();

		SwapChain.reset();

		if (ImGuiDescriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(LogicalDevice, ImGuiDescriptorPool, nullptr);

		vkDestroyDevice(LogicalDevice, nullptr);
		LogicalDevice = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(Instance->Instance, Surface, nullptr);
		Surface = VK_NULL_HANDLE;

		Instance.reset();
	}

	void VulkanContext::Begin() {
		vkWaitForFences(LogicalDevice, 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(LogicalDevice, SwapChain->GetHandle(), UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &CurrentImage);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			SwapChain->Recreate(Renderpass->GetHandle());
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			SE_FATAL(Logging::Rendering, "Failed to acquire swapchain image!");

		// only reset if we are submitting work
		vkResetFences(LogicalDevice, 1, &InFlightFences[CurrentFrame]);

		CommandBuffer = CommandPool->BeginRecording(CurrentFrame);

		SceneRenderpass->Begin(SceneRenderpass->GetFramebuffer(CurrentImage), CommandBuffer, VkExtent2D(SceneWidth, SceneHeight));
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


		Renderpass->Begin(SwapChain->GetFramebuffer(CurrentImage), CommandBuffer, SwapChain->GetExtent());
	}

	void VulkanContext::End() {
		Renderpass->End(CommandBuffer);

		CommandPool->EndRecording(CurrentFrame);

		// Submit
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &ImageAvailableSemaphores[CurrentFrame];
		VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &RenderFinishedSemaphores[CurrentFrame];
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &CommandBuffer;

		VkResult result = vkQueueSubmit(GraphicsQueue, 1, &submitInfo, InFlightFences[CurrentFrame]);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to submit commandBuffer");

		CommandBuffer = VK_NULL_HANDLE;

		// Present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentFrame];

		presentInfo.swapchainCount = 1;
		VkSwapchainKHR swapchain = SwapChain->GetHandle();
		presentInfo.pSwapchains = &swapchain;
		presentInfo.pImageIndices = &CurrentImage;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(PresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || FramebufferResized) {
			FramebufferResized = false;
			SwapChain->Recreate(Renderpass->GetHandle());
		}
		else if (result != VK_SUCCESS)
			SE_FATAL(Logging::Rendering, "Failed to present swapchain image!");

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
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkResult result = vkCreateSemaphore(LogicalDevice, &semaphoreInfo, nullptr, &ImageAvailableSemaphores[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create semaphore");
			result = vkCreateSemaphore(LogicalDevice, &semaphoreInfo, nullptr, &RenderFinishedSemaphores[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create semaphore");

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			result = vkCreateFence(LogicalDevice, &fenceInfo, nullptr, &InFlightFences[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create inFlightFence");
		}
	}

	void VulkanContext::_DestroySyncObjects() {
		for (uint32_t frameIndex = 0; frameIndex < MaxFramesInFlight; frameIndex++) {
			vkDestroySemaphore(LogicalDevice, ImageAvailableSemaphores[frameIndex], nullptr);
			vkDestroySemaphore(LogicalDevice, RenderFinishedSemaphores[frameIndex], nullptr);
			vkDestroyFence(LogicalDevice, InFlightFences[frameIndex], nullptr);
		}
	}
}