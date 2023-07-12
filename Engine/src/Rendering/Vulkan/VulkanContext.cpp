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

		vkDestroyDevice(m_LogicalDevice, nullptr);
		m_LogicalDevice = VK_NULL_HANDLE;

		vkDestroySurfaceKHR(m_Instance->Instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;

		m_Instance.reset();
	}

	void VulkanContext::Update() {
		vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);
		vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

		uint32_t imageIndex;
		vkAcquireNextImageKHR(m_LogicalDevice, m_SwapChain->GetHandle(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		VkCommandBuffer cmd = m_CommandPool->BeginRecording(m_CurrentFrame);

		m_Renderpass->Begin(cmd, imageIndex);

		m_TriangleShader->Bind(cmd);
		vkCmdDraw(cmd, 3, 1, 0, 0);

		m_Renderpass->End(cmd);

		m_CommandPool->EndRecording(m_CurrentFrame);

		// Submit
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		std::array<VkSemaphore, 1> waitSemaphores = { m_ImageAvailableSemaphores[m_CurrentFrame]};
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitStages;
		
		std::array<VkSemaphore, 1> signalSemaphores = { m_RenderFinishedSemaphores[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		submitInfo.pSignalSemaphores = signalSemaphores.data();
		
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VkResult result = vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to submit commandBuffer");


		// Present
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		presentInfo.pWaitSemaphores = signalSemaphores.data();

		VkSwapchainKHR swapChains[] = { m_SwapChain->GetHandle()};
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // Optional

		result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to present");


		m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFramesInFlight;
	}

	void VulkanContext::WaitBeforeClose() {
		vkDeviceWaitIdle(m_LogicalDevice);
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