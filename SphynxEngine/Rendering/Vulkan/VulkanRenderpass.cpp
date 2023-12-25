#include "pch.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	VulkanRenderpass::VulkanRenderpass(RenderPassUsage usage, vk::Format format, bool depth) : m_DepthEnabled(depth) {
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

		vk::AttachmentDescription depthAttachment{};
		depthAttachment.format = ChooseDepthFormat();
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::AttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		vk::SubpassDescription subpass{};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		if (depth)
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::vector<vk::AttachmentDescription> attachments = { colorAttachment };
		if (depth)
			attachments.push_back(depthAttachment);
		vk::RenderPassCreateInfo createInfo{};
		createInfo.attachmentCount = attachments.size();
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;

		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.srcAccessMask = (vk::AccessFlags)0;
		dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
		dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		vk::Result result = VulkanContext::LogicalDevice.createRenderPass(&createInfo, nullptr, &m_Renderpass);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create renderpass");
	}

	VulkanRenderpass::~VulkanRenderpass() {
		for (auto sceneFramebuffer : m_Framebuffers)
			VulkanContext::LogicalDevice.destroyFramebuffer(sceneFramebuffer, nullptr);

		for (auto imageView : m_ImageViews)
			VulkanContext::LogicalDevice.destroyImageView(imageView, nullptr);
		for (auto image : m_Images)
			VulkanContext::LogicalDevice.destroyImage(image, nullptr);
		for (auto memory : m_ImageMemories)
			VulkanContext::LogicalDevice.freeMemory(memory, nullptr);

		if (m_DepthEnabled) {
			VulkanContext::LogicalDevice.destroyImageView(m_DepthImageView, nullptr);
			VulkanContext::LogicalDevice.destroyImage(m_DepthImage, nullptr);
			VulkanContext::LogicalDevice.freeMemory(m_DepthImageMemory, nullptr);
		}

		VulkanContext::LogicalDevice.destroyRenderPass(m_Renderpass, nullptr);
		m_Renderpass = VK_NULL_HANDLE;
	}

	void VulkanRenderpass::CreateFramebuffers(uint32 width, uint32 height, vk::Format format, bool sampled) {
		m_Images.resize(VulkanContext::MaxFramesInFlight);
		m_ImageMemories.resize(VulkanContext::MaxFramesInFlight);
		m_ImageViews.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
			if (sampled)
				usage |= vk::ImageUsageFlagBits::eSampled;
			VulkanTexture::CreateImage(width, height, format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_Images[i], m_ImageMemories[i]);
			m_ImageViews[i] = VulkanTexture::CreateImageView(m_Images[i], format, vk::ImageAspectFlagBits::eColor);
		}

		if (m_DepthEnabled) {
			vk::Format depthFormat = ChooseDepthFormat();
			VulkanTexture::CreateImage(width, height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_DepthImage, m_DepthImageMemory);
			m_DepthImageView = VulkanTexture::CreateImageView(m_DepthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
		}

		m_Framebuffers.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = m_Renderpass;
			std::vector<vk::ImageView> attachments = { m_ImageViews[i] };
			if (m_DepthEnabled)
				attachments.push_back(m_DepthImageView);
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = width;
			framebufferInfo.height = height;
			framebufferInfo.layers = 1;

			vk::Result result = VulkanContext::LogicalDevice.createFramebuffer(&framebufferInfo, nullptr, &m_Framebuffers[i]);
			SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create scene framebuffer {}", i);
		}
	}

	vk::Framebuffer VulkanRenderpass::GetFramebuffer(uint32 imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_Framebuffers[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	vk::Image VulkanRenderpass::GetImage(uint32 imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_Images[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	vk::ImageView VulkanRenderpass::GetImageView(uint32 imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_ImageViews[imageIndex];
		else
			return VK_NULL_HANDLE;
	}

	void VulkanRenderpass::Begin(vk::Framebuffer framebuffer, vk::CommandBuffer commandBuffer, vk::Extent2D extent) {
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.renderPass = m_Renderpass;
		renderPassInfo.framebuffer = framebuffer;
		renderPassInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].color = vk::ClearColorValue{0.f, 0.f, 0.f, 1.f};
		clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.f, 0 };
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.data();
		
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


	vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		for (vk::Format format : candidates) {
			vk::FormatProperties props = VulkanContext::PhysicalDevice.getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	vk::Format VulkanRenderpass::ChooseDepthFormat() {
		return FindSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}
}