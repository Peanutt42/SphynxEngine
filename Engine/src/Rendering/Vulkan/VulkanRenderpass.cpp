#include "pch.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanRenderpass::VulkanRenderpass(RenderPassUsage usage, vk::Format format) {
		vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout finalLayout = vk::ImageLayout::eUndefined;
		switch (usage) {
		default: break;
		case RenderPassUsage::Single:
			initialLayout = vk::ImageLayout::eUndefined;
			finalLayout = vk::ImageLayout::ePresentSrcKHR;
			break;
		case RenderPassUsage::First:
			initialLayout = vk::ImageLayout::eUndefined;
			finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
			break;
		case RenderPassUsage::Middle:
			initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
			finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
			break;
		case RenderPassUsage::Last:
			initialLayout = vk::ImageLayout::eUndefined;
			finalLayout = vk::ImageLayout::ePresentSrcKHR;
			break;
		}

		vk::AttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = initialLayout;
		colorAttachment.finalLayout = finalLayout;

		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		vk::SubpassDescription subpass{};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		vk::RenderPassCreateInfo createInfo{};
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &colorAttachment;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;

		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.srcAccessMask = (vk::AccessFlags)0;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		vk::Result result = VulkanContext::LogicalDevice.createRenderPass(&createInfo, nullptr, &m_Renderpass);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create renderpass");
	}

	VulkanRenderpass::~VulkanRenderpass() {
		for (auto sceneFramebuffer : m_Framebuffers)
			VulkanContext::LogicalDevice.destroyFramebuffer(sceneFramebuffer, nullptr);

		for (auto imageView : m_FramebufferImageViews)
			VulkanContext::LogicalDevice.destroyImageView(imageView, nullptr);
		for (auto image : m_FramebufferImages)
			VulkanContext::LogicalDevice.destroyImage(image, nullptr);
		for (auto memory : m_FramebufferImageMemories)
			VulkanContext::LogicalDevice.freeMemory(memory, nullptr);

		VulkanContext::LogicalDevice.destroyRenderPass(m_Renderpass, nullptr);
		m_Renderpass = VK_NULL_HANDLE;
	}

	void VulkanRenderpass::CreateFramebuffers(uint32_t width, uint32_t height, vk::Format format, bool sampled) {
		m_FramebufferImages.resize(VulkanContext::MaxFramesInFlight);
		m_FramebufferImageMemories.resize(VulkanContext::MaxFramesInFlight);
		m_FramebufferImageViews.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
			if (sampled)
				usage |= vk::ImageUsageFlagBits::eSampled;
			VulkanTexture::CreateImage(width, height, format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_FramebufferImages[i], m_FramebufferImageMemories[i]);
			m_FramebufferImageViews[i] = VulkanTexture::CreateImageView(m_FramebufferImages[i], format, vk::ImageAspectFlagBits::eColor);
		}

		m_Framebuffers.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = m_Renderpass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &m_FramebufferImageViews[i];
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			vk::Result result = VulkanContext::LogicalDevice.createFramebuffer(&framebufferInfo, nullptr, &m_Framebuffers[i]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create scene framebuffer {}", i);
		}
	}

	vk::Framebuffer VulkanRenderpass::GetFramebuffer(uint32_t imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_Framebuffers[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	vk::Image VulkanRenderpass::GetImage(uint32_t imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_FramebufferImages[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	vk::ImageView VulkanRenderpass::GetImageView(uint32_t imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_FramebufferImageViews[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	void VulkanRenderpass::Begin(vk::Framebuffer framebuffer, vk::CommandBuffer commandBuffer, vk::Extent2D extent) {
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = m_Renderpass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		vk::ClearValue clearColor;
		clearColor.color = vk::ClearColorValue(0.f, 0.f, 0.f, 1.f);
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		
		commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);


		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandBuffer.setViewport(0, 1, &viewport);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = extent;
		commandBuffer.setScissor(0, 1, &scissor);
	}

	void VulkanRenderpass::End(vk::CommandBuffer commandBuffer) {
		commandBuffer.endRenderPass();
	}
}