#include "pch.hpp"
#include "VulkanContext.hpp"
#include "VulkanDevice.hpp"
#include "VulkanSurface.hpp"

namespace Sphynx::Rendering {
	VulkanContext::VulkanContext(Window& window)
		: m_Window(window)
	{
		m_Instance = std::make_unique<VulkanInstance>(
#if defined(DEBUG) || defined(RELEASE)
			true
#else
			false
#endif
		);

		m_Surface = VulkanSurface::GetFromWindow(m_Instance->Instance, window.GetGLFWHandle());

		std::vector<const char*> deviceExtensions {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		m_PhysicalDevice = VulkanPhysicalDevice::Pick(m_Instance->Instance, m_Surface, deviceExtensions);
		
		SE_INFO(Logging::Rendering, "Chosen GPU: {}", VulkanPhysicalDevice::GetName(m_PhysicalDevice));

		VulkanLogicalDevice::CreateResult logicalDeviceResult = VulkanLogicalDevice::Create(m_PhysicalDevice, m_Instance->ValidationLayers, deviceExtensions, m_Surface);
		m_LogicalDevice = logicalDeviceResult.Device;
		m_GraphicsQueue = logicalDeviceResult.GraphicsQueue;
		m_GraphicsQueue = logicalDeviceResult.PresentQueue;
		m_PresentQueue = logicalDeviceResult.PresentQueue;

		m_SwapChain = std::make_unique<VulkanSwapChain>(m_PhysicalDevice, m_LogicalDevice, m_Surface, m_Window.GetGLFWHandle());

		m_Renderpass = std::make_unique<VulkanRenderpass>(m_LogicalDevice, *m_SwapChain);

		ShaderCreateInfo createInfo("Engine/Resources/Shaders/triangle.glsl");
		m_TriangleShader = std::make_unique<VulkanShader>(createInfo, m_LogicalDevice, *m_Renderpass);

		m_SwapChain->CreateFramebuffers(m_Renderpass->GetHandle());

		m_CommandPool = std::make_unique<VulkanCommandPool>(m_PhysicalDevice, m_LogicalDevice, m_Surface, m_MaxFramesInFlight);

		_CreateSyncObjects();
	}

	VulkanContext::~VulkanContext() {
		_DestroySyncObjects();

		m_CommandPool.reset();

		m_TriangleShader.reset();

		m_Renderpass.reset();

		m_SwapChain.reset();

		if (m_ImGuiDescriptorPool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(m_LogicalDevice, m_ImGuiDescriptorPool, nullptr);

		vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(m_Instance->Instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;

		m_Instance.reset();
	}

	void VulkanContext::Begin() {
		vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		VkResult result = vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain->GetHandle(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_CurrentImage);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			m_SwapChain->Recreate(m_Renderpass->GetHandle());
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			SE_FATAL(Logging::Rendering, "Failed to acquire swapchain image!");

		// only reset if we are submitting work
		vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		m_CommandBuffer = m_CommandPool->BeginRecording(m_CurrentFrame);

		m_Renderpass->Begin(m_CommandBuffer, m_CurrentImage);
	}

	void VulkanContext::End() {
		m_Renderpass->End(m_CommandBuffer);

		m_CommandPool->EndRecording(m_CurrentFrame);

		// Submit
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::array<VkSemaphore, 1> waitSemaphores = { m_ImageAvailableSemaphores[m_CurrentFrame] };
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();

		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitStages;

		std::array<VkSemaphore, 1> signalSemaphores = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_CommandBuffer;

		VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to submit commandBuffer");

		m_CommandBuffer = VK_NULL_HANDLE;

		// Present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		presentInfo.pWaitSemaphores = signalSemaphores.data();

		VkSwapchainKHR swapChains[] = { m_SwapChain->GetHandle() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_CurrentImage;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized) {
			m_FramebufferResized = false;
			m_SwapChain->Recreate(m_Renderpass->GetHandle());
		}
		else if (result != VK_SUCCESS)
			SE_FATAL(Logging::Rendering, "Failed to present swapchain image!");

		m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
	}

	void VulkanContext::WaitBeforeClose() {
		vkDeviceWaitIdle(m_LogicalDevice);
	}

	VkDescriptorPool VulkanContext::GetImGuiDescriptorPool() {
		if (m_ImGuiDescriptorPool == VK_NULL_HANDLE) {
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

			vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_ImGuiDescriptorPool);
		}
		return m_ImGuiDescriptorPool;
	}

	void VulkanContext::_CreateSyncObjects() {
		m_ImageAvailableSemaphores.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
		m_RenderFinishedSemaphores.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);
		m_InFlightFences.resize(m_MaxFramesInFlight, VK_NULL_HANDLE);

		for (uint32_t frameIndex = 0; frameIndex < m_MaxFramesInFlight; frameIndex++) {
			VkSemaphoreCreateInfo semaphoreInfo{};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			VkResult result = vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create semaphore");
			result = vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create semaphore");

			VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			result = vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFences[frameIndex]);
			SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create inFlightFence");
		}
	}

	void VulkanContext::_DestroySyncObjects() {
		for (uint32_t frameIndex = 0; frameIndex < m_MaxFramesInFlight; frameIndex++) {
			vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[frameIndex], nullptr);
			vkDestroySemaphore(m_LogicalDevice, m_RenderFinishedSemaphores[frameIndex], nullptr);
			vkDestroyFence(m_LogicalDevice, m_InFlightFences[frameIndex], nullptr);
		}
	}
}