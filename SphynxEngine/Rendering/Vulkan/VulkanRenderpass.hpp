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

	class VulkanRenderpass {
	public:
		VulkanRenderpass(RenderPassUsage usage, vk::Format format, bool depth);
		~VulkanRenderpass();

		vk::RenderPass GetHandle() { return m_Renderpass; }

		// Isn't needed for the swapchain renderpass
		void CreateFramebuffers(uint32 width, uint32 height, vk::Format format, bool sampled);

		vk::Framebuffer GetFramebuffer(uint32 imageIndex);
		vk::Image GetImage(uint32 imageIndex);
		vk::ImageView GetImageView(uint32 imageIndex);
		const std::vector<vk::ImageView>& GetImageViews() { return m_ImageViews; }

		void Begin(vk::Framebuffer framebuffer, vk::CommandBuffer commandBuffer, vk::Extent2D extent);
		void End(vk::CommandBuffer commandBuffer);

	private:
		VulkanRenderpass(const VulkanRenderpass&) = delete;
		VulkanRenderpass(VulkanRenderpass&&) = delete;
		VulkanRenderpass& operator=(const VulkanRenderpass&) = delete;
		VulkanRenderpass& operator=(VulkanRenderpass&&) = delete;

		static vk::Format ChooseDepthFormat();

	private:
		vk::RenderPass m_Renderpass;

		// own framebuffers
		std::vector<vk::Framebuffer> m_Framebuffers;

		std::vector<vk::Image> m_Images;
		std::vector<vk::DeviceMemory> m_ImageMemories;
		std::vector<vk::ImageView> m_ImageViews;

		bool m_DepthEnabled = false;
		vk::Image m_DepthImage;
		vk::DeviceMemory m_DepthImageMemory;
		vk::ImageView m_DepthImageView;
	};
}