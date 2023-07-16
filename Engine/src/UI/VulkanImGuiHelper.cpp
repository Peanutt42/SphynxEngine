#include "pch.hpp"
#include "VulkanImGuiHelper.hpp"
#include "Fonts.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace Sphynx::UI {
	VulkanImGuiHelper::VulkanImGuiHelper(Rendering::VulkanContext& context) {
		SE_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;

		Fonts::Init();

		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(context.GetWindowHandle(), true);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = context.GetVkInstance();
		init_info.PhysicalDevice = context.GetPhysicalDevice();
		init_info.Device = context.GetDevice();
		init_info.Queue = context.GetGraphicsQueue();
		init_info.DescriptorPool = context.GetImGuiDescriptorPool();
		init_info.MinImageCount = context.GetMaxFramesInFlight();
		init_info.ImageCount = context.GetMaxFramesInFlight();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, context.GetRenderpass().GetHandle());

		VkCommandBuffer cmd = context.GetCommandPool().BeginSingleUseCommandbuffer();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		context.GetCommandPool().EndSingleUseCommandbuffer(cmd, context);

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	VulkanImGuiHelper::~VulkanImGuiHelper() {
		ImGui_ImplVulkan_Shutdown();

		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}

	void VulkanImGuiHelper::Begin() {
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}

	void VulkanImGuiHelper::End() {
		ImGui::Render();
	}

	void VulkanImGuiHelper::Render(VkCommandBuffer cmd) {
		ImDrawData* drawData = ImGui::GetDrawData();
		if (drawData)
			ImGui_ImplVulkan_RenderDrawData(drawData, cmd);
	}

	void VulkanImGuiHelper::EnableDocking() {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void VulkanImGuiHelper::BeginDockspace(const std::string& name) {
		if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
			return;

		constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		if constexpr (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		static bool opened = true;
		ImGui::Begin(name.data(), &opened, window_flags);

		ImGui::PopStyleVar();
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
			ImGuiID dockspace_id = ImGui::GetID(name.data());
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
		}
	}

	void VulkanImGuiHelper::EndDockspace() {
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
		ImGui::End();
	}

	void VulkanImGuiHelper::SetSaveFilepath(const std::filesystem::path& filepath) {
		static std::string filepathStr; // Ensures lifetime of the string
		filepathStr = filepath.string();
		ImGui::GetIO().IniFilename = filepathStr.c_str();
	}

	void VulkanImGuiHelper::DisableSaveFile() {
		ImGui::GetIO().IniFilename = nullptr;
	}

	ImGuiContext* VulkanImGuiHelper::GetContext() {
		return ImGui::GetCurrentContext();
	}
}