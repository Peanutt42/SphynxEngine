#include "pch.hpp"
#include "VulkanImGui.hpp"
#include "Core/Engine.hpp"
#include "Fonts.hpp"
#include "UIThemes.hpp"
#include "UI.hpp"
#include "Rendering/Vulkan/VulkanContext.hpp"

#include "Profiling/Profiling.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <GLFW/glfw3.h>

namespace Sphynx::UI {
	void VulkanImGui::Init() {
		SE_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		Fonts::Init();

		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(Rendering::VulkanContext::Window->GetGLFWHandle(), true);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = Rendering::VulkanContext::Instance->Instance;
		init_info.PhysicalDevice = Rendering::VulkanContext::PhysicalDevice;
		init_info.Device = Rendering::VulkanContext::LogicalDevice;
		init_info.Queue = Rendering::VulkanContext::GraphicsQueue;
		init_info.DescriptorPool = Rendering::VulkanContext::DescriptorPool;
		init_info.MinImageCount = Rendering::VulkanContext::MaxFramesInFlight;
		init_info.ImageCount = Rendering::VulkanContext::MaxFramesInFlight;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.PipelineCache = VK_NULL_HANDLE;

		ImGui_ImplVulkan_Init(&init_info, Rendering::VulkanContext::Renderpass->GetHandle());

		VkCommandBuffer cmd = Rendering::VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		Rendering::VulkanContext::CommandPool->EndSingleUseCommandbuffer(cmd);

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		Themes::SetDefaultTheme();
	}

	void VulkanImGui::Shutdown() {
		ImGui_ImplVulkan_Shutdown();

		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}

	void VulkanImGui::Begin() {
		SE_PROFILE_FUNCTION();

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();


		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
											ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			if constexpr (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			bool isMaximized = Rendering::VulkanContext::Window->IsMaximized();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
			static bool opened = true;
			ImGui::Begin("Dockspace", &opened, window_flags);

			ImGui::PopStyleVar(2);

			ImGuiID dockspace_id = ImGui::GetID("Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
		}
	}

	void VulkanImGui::End() {
		SE_PROFILE_FUNCTION();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
			ImGui::End();

		ImGui::Render();
	}

	
	void VulkanImGui::Render() {
		SE_PROFILE_FUNCTION();

		ImDrawData* drawData = ImGui::GetDrawData();
		if (drawData)
			ImGui_ImplVulkan_RenderDrawData(drawData, Rendering::VulkanContext::CommandBuffer);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanImGui::EnableDocking() {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void VulkanImGui::SetSaveFilepath(const std::filesystem::path& filepath) {
		static std::string filepathStr; // Ensures lifetime of the string
		filepathStr = filepath.string();
		ImGui::GetIO().IniFilename = filepathStr.c_str();
	}

	void VulkanImGui::DisableSaveFile() {
		ImGui::GetIO().IniFilename = nullptr;
	}

	ImGuiContext* VulkanImGui::GetContext() {
		return ImGui::GetCurrentContext();
	}
}