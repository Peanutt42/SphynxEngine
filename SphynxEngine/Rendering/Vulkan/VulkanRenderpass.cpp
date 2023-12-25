#include "pch.hpp"
#include "VulkanRenderpass.hpp"
#include "VulkanContext.hpp"

namespace Sphynx::Rendering {
	ColorAttachment::~ColorAttachment() {
		for (auto imageView : m_ImageViews)
			VulkanContext::LogicalDevice.destroyImageView(imageView, nullptr);
		for (auto image : m_Images)
			VulkanContext::LogicalDevice.destroyImage(image, nullptr);
		for (auto memory : m_ImageMemories)
			VulkanContext::LogicalDevice.freeMemory(memory, nullptr);
	}
	
	vk::AttachmentDescription ColorAttachment::GetDescription(vk::ImageLayout initialColorLayout, vk::ImageLayout finalColorLayout) const {
		vk::AttachmentDescription colorAttachment{};
		colorAttachment.format = m_Format;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = initialColorLayout;
		colorAttachment.finalLayout = finalColorLayout;
		return colorAttachment;
	}

	vk::AttachmentReference ColorAttachment::GetReference() const {
		vk::AttachmentReference colorAttachmentRef{};
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
		return colorAttachmentRef;
	}

	vk::PipelineStageFlagBits ColorAttachment::GetPipelineStage() const {
		return vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}

	vk::AccessFlagBits ColorAttachment::GetWriteAccess() const {
		return vk::AccessFlagBits::eColorAttachmentWrite;
	}

	void ColorAttachment::ConfigureSubpass(vk::SubpassDescription& subpass, vk::AttachmentReference& reference) const {
		// TODO: Add support for multiple color attachments
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &reference;
	}

	void ColorAttachment::CreateImages(int width, int height) {
		m_Images.resize(VulkanContext::MaxFramesInFlight);
		m_ImageMemories.resize(VulkanContext::MaxFramesInFlight);
		m_ImageViews.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
			if (m_Sampled)
				usage |= vk::ImageUsageFlagBits::eSampled;
			VulkanTexture::CreateImage(width, height, m_Format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal, m_Images[i], m_ImageMemories[i]);
			m_ImageViews[i] = VulkanTexture::CreateImageView(m_Images[i], m_Format, vk::ImageAspectFlagBits::eColor);
		}
	}

	vk::Image ColorAttachment::GetImage(int index) const {
		return m_Images[index];
	}

	vk::ImageView ColorAttachment::GetImageView(int index) const {
		return m_ImageViews[index];
	}


	DepthAttachment::DepthAttachment() {
		m_Format = VulkanRenderpass::ChooseDepthFormat();
	}

	DepthAttachment::~DepthAttachment() {
		VulkanContext::LogicalDevice.destroyImageView(m_ImageView, nullptr);
		VulkanContext::LogicalDevice.destroyImage(m_Image, nullptr);
		VulkanContext::LogicalDevice.freeMemory(m_ImageMemory, nullptr);
	}

	vk::AttachmentDescription DepthAttachment::GetDescription(vk::ImageLayout, vk::ImageLayout) const {
		vk::AttachmentDescription depthAttachment{};
		depthAttachment.format = m_Format;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
		depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		return depthAttachment;
	}

	vk::AttachmentReference DepthAttachment::GetReference() const {
		vk::AttachmentReference depthAttachmentRef{};
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		return depthAttachmentRef;
	}

	vk::PipelineStageFlagBits DepthAttachment::GetPipelineStage() const {
		return vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}

	vk::AccessFlagBits DepthAttachment::GetWriteAccess() const {
		return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
	}

	void DepthAttachment::ConfigureSubpass(vk::SubpassDescription& subpass, vk::AttachmentReference& reference) const {
		subpass.pDepthStencilAttachment = &reference;
	}

	void DepthAttachment::CreateImages(int width, int height) {
		VulkanTexture::CreateImage(width, height, m_Format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_Image, m_ImageMemory);
		m_ImageView = VulkanTexture::CreateImageView(m_Image, m_Format, vk::ImageAspectFlagBits::eDepth);
	}

	vk::Image DepthAttachment::GetImage(int) const {
		return m_Image;
	}

	vk::ImageView DepthAttachment::GetImageView(int) const {
		return m_ImageView;
	}

	VulkanRenderpass::VulkanRenderpass(RenderPassUsage usage, const std::vector<std::shared_ptr<Attachment>>& attachments)
		: m_Attachments(attachments)
	{
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
		
		std::vector<vk::AttachmentDescription> attachmentDescriptions;
		std::vector<vk::AttachmentReference> attachmentReferences;
		for (size_t i = 0; i < m_Attachments.size(); i++) {
			attachmentDescriptions.push_back(m_Attachments[i]->GetDescription(initialLayout, finalLayout));

			auto reference = m_Attachments[i]->GetReference();
			reference.attachment = i;
			attachmentReferences.push_back(reference);
		}

		vk::SubpassDescription subpass{};
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		for (size_t i = 0; i < m_Attachments.size(); i++)
			m_Attachments[i]->ConfigureSubpass(subpass, attachmentReferences[i]);
		
		vk::RenderPassCreateInfo createInfo{};
		createInfo.attachmentCount = attachmentDescriptions.size();
		createInfo.pAttachments = attachmentDescriptions.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;

		vk::SubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcAccessMask = (vk::AccessFlags)0;
		for (const auto& attachment : m_Attachments) {
			auto pipelineStage = attachment->GetPipelineStage();
			dependency.srcStageMask |= pipelineStage;
			dependency.dstStageMask |= pipelineStage;
			dependency.dstAccessMask |= attachment->GetWriteAccess();
		}

		createInfo.dependencyCount = 1;
		createInfo.pDependencies = &dependency;

		vk::Result result = VulkanContext::LogicalDevice.createRenderPass(&createInfo, nullptr, &m_Renderpass);
		SE_ASSERT(result == vk::Result::eSuccess, Logging::Rendering, "Failed to create renderpass");
	}

	VulkanRenderpass::~VulkanRenderpass() {
		for (auto sceneFramebuffer : m_Framebuffers)
			VulkanContext::LogicalDevice.destroyFramebuffer(sceneFramebuffer, nullptr);

		m_Attachments.clear();

		VulkanContext::LogicalDevice.destroyRenderPass(m_Renderpass, nullptr);
		m_Renderpass = VK_NULL_HANDLE;
	}

	void VulkanRenderpass::CreateFramebuffers(uint32 width, uint32 height) {
		for (auto& attachment : m_Attachments)
			attachment->CreateImages(width, height);

		m_Framebuffers.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++) {
			vk::FramebufferCreateInfo framebufferInfo{};
			framebufferInfo.renderPass = m_Renderpass;
			std::vector<vk::ImageView> attachmentViews;
			for (const auto& attachment : m_Attachments)
				attachmentViews.push_back(attachment->GetImageView(i));
			framebufferInfo.attachmentCount = attachmentViews.size();
			framebufferInfo.pAttachments = attachmentViews.data();
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

	vk::Image VulkanRenderpass::GetImage(int attachmentIndex, uint32 imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_Attachments[attachmentIndex]->GetImage(imageIndex);
		else
			return VK_NULL_HANDLE;
	}

	vk::ImageView VulkanRenderpass::GetImageView(int attachmentIndex, uint32 imageIndex) {
		if (imageIndex < m_Framebuffers.size())
			return m_Attachments[attachmentIndex]->GetImageView(imageIndex);
		else
			return VK_NULL_HANDLE;
	}

	std::vector<vk::ImageView> VulkanRenderpass::GetImageViews(int attachmentIndex) {
		std::vector<vk::ImageView> views;
		views.resize(VulkanContext::MaxFramesInFlight);
		for (size_t i = 0; i < VulkanContext::MaxFramesInFlight; i++)
			views[i] = m_Attachments[attachmentIndex]->GetImageView(i);
		return views;
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