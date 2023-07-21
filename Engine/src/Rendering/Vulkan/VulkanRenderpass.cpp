#include "pch.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanRenderpass::VulkanRenderpass(RenderPassUsage usage, VkDevice device, VkFormat format)
		: m_Device(device)
	{
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		switch (usage) {
		default: break;
		case RenderPassUsage::Single:
			initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		case RenderPassUsage::First:
			initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case RenderPassUsage::Middle:
			initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case RenderPassUsage::Last:
			initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			break;
		}

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = initialLayout;
		colorAttachment.finalLayout = finalLayout;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(m_Device, &createInfo, nullptr, &m_Renderpass);
		SE_ASSERT(result == VK_SUCCESS, Logging::Rendering, "Failed to create renderpass");
	}

	VulkanRenderpass::~VulkanRenderpass() {
		for (auto sceneFramebuffer : m_Framebuffers)
			vkDestroyFramebuffer(m_Device, sceneFramebuffer, nullptr);

		for (auto imageView : m_FramebufferImageViews)
			vkDestroyImageView(m_Device, imageView, nullptr);
		for (auto image : m_FramebufferImages)
			vkDestroyImage(m_Device, image, nullptr);
		for (auto memory : m_FramebufferImageMemories)
			vkFreeMemory(m_Device, memory, nullptr);

		vkDestroyRenderPass(m_Device, m_Renderpass, nullptr);
		m_Renderpass = VK_NULL_HANDLE;
	}

	void VulkanRenderpass::CreateFramebuffers(VkPhysicalDevice physicalDevice, uint32_t maxFramesInFlight, uint32_t width, uint32_t height, VkFormat format, VkSharingMode sharingMode) {
		m_FramebufferImages.resize(maxFramesInFlight);
		m_FramebufferImageMemories.resize(maxFramesInFlight);
		m_FramebufferImageViews.resize(maxFramesInFlight);
		for (size_t i = 0; i < maxFramesInFlight; i++) {
			VulkanTexture::CreateImage(physicalDevice, m_Device, width, height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sharingMode, m_FramebufferImages[i], m_FramebufferImageMemories[i]);
			m_FramebufferImageViews[i] = VulkanTexture::CreateImageView(m_Device, m_FramebufferImages[i], format, VK_IMAGE_ASPECT_COLOR_BIT);
		}

		m_Framebuffers.resize(maxFramesInFlight);
		for (size_t i = 0; i < maxFramesInFlight; i++) {
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_Renderpass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &m_FramebufferImageViews[i];
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS)
				SE_ERR(Logging::Rendering, "Failed to create scene framebuffer {}", i);
		}
	}

	VkFramebuffer VulkanRenderpass::GetFramebuffer(uint32_t currentImageIndex) {
		if (currentImageIndex < m_Framebuffers.size())
			return m_Framebuffers[currentImageIndex];
		else
			return VK_NULL_HANDLE;
	}

	void VulkanRenderpass::Begin(VkFramebuffer framebuffer, VkCommandBuffer commandBuffer, VkExtent2D extent) {
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Renderpass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderpass::End(VkCommandBuffer commandBuffer) {
		vkCmdEndRenderPass(commandBuffer);
	}
}