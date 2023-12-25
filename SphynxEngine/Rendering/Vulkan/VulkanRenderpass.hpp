#pragma once

#include "pch.hpp"
#include "VulkanSwapChain.hpp"

namespace Sphynx::Rendering {
	enum class RenderPassUsage {
		Single,
		First,
		Middle,
		Last
	};

	class Attachment {
	public:
		virtual vk::AttachmentDescription GetDescription(vk::ImageLayout initialColorLayout, vk::ImageLayout finalColorLayout) const = 0;
		virtual vk::AttachmentReference GetReference() const = 0;
		virtual vk::PipelineStageFlagBits GetPipelineStage() const = 0;
		virtual vk::AccessFlagBits GetWriteAccess() const = 0;
		virtual void ConfigureSubpass(vk::SubpassDescription& subpass, vk::AttachmentReference& reference) const = 0;
		
		virtual void CreateImages(int width, int height) = 0;
		virtual vk::Image GetImage(int index) const = 0;
		virtual vk::ImageView GetImageView(int index) const = 0;
	};

	class ColorAttachment : public Attachment {
	public:
		ColorAttachment(vk::Format format, bool sampled) : m_Format(format), m_Sampled(sampled) {}
		~ColorAttachment();

		virtual vk::AttachmentDescription GetDescription(vk::ImageLayout initialColorLayout, vk::ImageLayout finalColorLayout) const override;
		virtual vk::AttachmentReference GetReference() const override;
		virtual vk::PipelineStageFlagBits GetPipelineStage() const override;
		virtual vk::AccessFlagBits GetWriteAccess() const override;
		virtual void ConfigureSubpass(vk::SubpassDescription& subpass, vk::AttachmentReference& reference) const override;
		
		virtual void CreateImages(int width, int height) override;
		virtual vk::Image GetImage(int index) const override;
		virtual vk::ImageView GetImageView(int index) const override;

	private:
		vk::Format m_Format;
		bool m_Sampled = false;
		std::vector<vk::Image> m_Images;
		std::vector<vk::DeviceMemory> m_ImageMemories;
		std::vector<vk::ImageView> m_ImageViews;
	};

	class DepthAttachment : public Attachment {
	public:
		DepthAttachment();
		~DepthAttachment();

		virtual vk::AttachmentDescription GetDescription(vk::ImageLayout, vk::ImageLayout) const override;
		virtual vk::AttachmentReference GetReference() const override;
		virtual vk::PipelineStageFlagBits GetPipelineStage() const override;
		virtual vk::AccessFlagBits GetWriteAccess() const override;
		virtual void ConfigureSubpass(vk::SubpassDescription& subpass, vk::AttachmentReference& reference) const override;
		
		virtual void CreateImages(int width, int height) override;
		virtual vk::Image GetImage(int) const override;
		virtual vk::ImageView GetImageView(int) const override;

	private:
		vk::Format m_Format;
		vk::Image m_Image;
		vk::DeviceMemory m_ImageMemory;
		vk::ImageView m_ImageView;
	};

	class VulkanRenderpass {
	public:
		VulkanRenderpass(RenderPassUsage usage, const std::vector<std::shared_ptr<Attachment>>& attachments);
		~VulkanRenderpass();

		vk::RenderPass GetHandle() { return m_Renderpass; }

		// Isn't needed for the swapchain renderpass
		void CreateFramebuffers(uint32 width, uint32 height);

		vk::Framebuffer GetFramebuffer(uint32 imageIndex);
		vk::Image GetImage(int attachmentIndex, uint32 imageIndex);
		vk::ImageView GetImageView(int attachmentIndex, uint32 imageIndex);
		std::vector<vk::ImageView> GetImageViews(int attachmentIndex);

		void Begin(vk::Framebuffer framebuffer, vk::CommandBuffer commandBuffer, vk::Extent2D extent);
		void End(vk::CommandBuffer commandBuffer);

		static vk::Format ChooseDepthFormat();

	private:
		VulkanRenderpass(const VulkanRenderpass&) = delete;
		VulkanRenderpass(VulkanRenderpass&&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&) = delete;
		VulkanRenderpass& operator=(VulkanRenderpass&&) = delete;

	private:
		vk::RenderPass m_Renderpass;

		// own framebuffers
		std::vector<vk::Framebuffer> m_Framebuffers;

		std::vector<std::shared_ptr<Attachment>> m_Attachments;
	};
}