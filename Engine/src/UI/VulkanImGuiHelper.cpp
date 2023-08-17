#include "pch.hpp"
#include "VulkanImGuiHelper.hpp"
#include "Core/Engine.hpp"
#include "Fonts.hpp"
#include "UIThemes.hpp"
#include "UI.hpp"
#include "Rendering/Vulkan/VulkanContext.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <GLFW/glfw3.h>

#include "../Resources/Icons/WindowToolbarIcons.embed"

namespace Sphynx::UI {
	VulkanImGuiHelper::VulkanImGuiHelper() {
		SE_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigViewportsNoTaskBarIcon = true;

		Fonts::Init();

		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(Rendering::VulkanContext::Window->GetGLFWHandle(), true);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = Rendering::VulkanContext::Instance->Instance;
		init_info.PhysicalDevice = Rendering::VulkanContext::PhysicalDevice;
		init_info.Device = Rendering::VulkanContext::LogicalDevice;
		init_info.Queue = Rendering::VulkanContext::GraphicsQueue;
		init_info.DescriptorPool = Rendering::VulkanContext::ImGuiDescriptorPool;
		init_info.MinImageCount = Rendering::VulkanContext::MaxFramesInFlight;
		init_info.ImageCount = Rendering::VulkanContext::MaxFramesInFlight;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		ImGui_ImplVulkan_Init(&init_info, Rendering::VulkanContext::Renderpass->GetHandle());

		VkCommandBuffer cmd = Rendering::VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		Rendering::VulkanContext::CommandPool->EndSingleUseCommandbuffer(cmd);

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		Themes::SetDefaultTheme();

		Rendering::VulkanContext::Window->SetTitlebarhitTestCallback([this]() -> bool {
			return m_TitlebarHovered;
		});


		// Load icons
		m_MinimizeIcon = std::make_unique<Rendering::Image>(BufferView(g_WindowMinimizeIcon));
		m_MaximizeIcon = std::make_unique<Rendering::Image>(BufferView(g_WindowMaximizeIcon));
		m_RestoreIcon = std::make_unique<Rendering::Image>(BufferView(g_WindowRestoreIcon));
		m_CloseIcon = std::make_unique<Rendering::Image>(BufferView(g_WindowCloseIcon));
	}

	VulkanImGuiHelper::~VulkanImGuiHelper() {
		m_MinimizeIcon.reset();
		m_MaximizeIcon.reset();
		m_RestoreIcon.reset();
		m_CloseIcon.reset();

		ImGui_ImplVulkan_Shutdown();

		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}

	void VulkanImGuiHelper::Begin() {
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

			_DrawTitlebar();


			ImGuiID dockspace_id = ImGui::GetID("Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
		}
	}

	void VulkanImGuiHelper::End() {
		SE_PROFILE_FUNCTION();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
			ImGui::End();

		ImGui::Render();
	}

	
	void VulkanImGuiHelper::Render() {
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

	void VulkanImGuiHelper::EnableDocking() {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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

	void VulkanImGuiHelper::_DrawTitlebar() {
		SE_PROFILE_FUNCTION();

		if (Engine::GetSettings().CustomWindowControls) {
			const float titlebarHeight = 37.f;
			const bool isMaximized = Rendering::VulkanContext::Window->IsMaximized();
			float titlebarVerticalOffset = isMaximized ? -6.0f : 0.0f;
			const ImVec2 windowPadding = ImGui::GetCurrentWindow()->WindowPadding;

			ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset));
			const ImVec2 titlebarMin = ImGui::GetCursorScreenPos();
			const ImVec2 titlebarMax = { ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth() - windowPadding.y * 2.0f,
										 ImGui::GetCursorScreenPos().y + titlebarHeight };
			auto* bgDrawList = ImGui::GetBackgroundDrawList();
			bgDrawList->AddRectFilled(titlebarMin, titlebarMax, Themes::Default::titlebar);
			// DEBUG TITLEBAR BOUNDS
			// auto* fgDrawList = ImGui::GetForegroundDrawList();
			// fgDrawList->AddRect(titlebarMin, titlebarMax, IM_COL32(255, 0, 0, 255));

			//// Logo
			//{
			//	const int logoWidth = 48;// m_LogoTex->GetWidth();
			//	const int logoHeight = 48;// m_LogoTex->GetHeight();
			//	const ImVec2 logoOffset(16.0f + windowPadding.x, 5.0f + windowPadding.y + titlebarVerticalOffset);
			//	const ImVec2 logoRectStart = { ImGui::GetItemRectMin().x + logoOffset.x, ImGui::GetItemRectMin().y + logoOffset.y };
			//	const ImVec2 logoRectMax = { logoRectStart.x + logoWidth, logoRectStart.y + logoHeight };
			//	fgDrawList->AddImage(m_AppHeaderIcon->GetDescriptorSet(), logoRectStart, logoRectMax);
			//}

			ImGui::BeginHorizontal("Titlebar", { ImGui::GetWindowWidth() - windowPadding.y * 2.0f, ImGui::GetFrameHeightWithSpacing() });

			static float moveOffsetX;
			static float moveOffsetY;
			const float w = ImGui::GetContentRegionAvail().x;
			const float buttonsAreaWidth = 94;

			// Title bar drag area
			// On Windows we hook into the GLFW win32 window internals
			ImGui::SetCursorPos(ImVec2(windowPadding.x, windowPadding.y + titlebarVerticalOffset)); // Reset cursor pos
			// DEBUG DRAG BOUNDS
			// fgDrawList->AddRect(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + w - buttonsAreaWidth, ImGui::GetCursorScreenPos().y + titlebarHeight), IM_COL32(255, 0, 0, 255));
			ImGui::InvisibleButton("##titleBarDragZone", ImVec2(w - buttonsAreaWidth, titlebarHeight));

			m_TitlebarHovered = ImGui::IsItemHovered();

			if (isMaximized)
			{
				float windowMousePosY = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
				if (windowMousePosY >= 0.0f && windowMousePosY <= 5.0f)
					m_TitlebarHovered = true; // Account for the top-most pixels which don't register
			}

			// Draw Menubar
			if (m_MenubarCallback) {
				ImGui::SuspendLayout();

				ImGui::SetItemAllowOverlap();
				ImGui::SetCursorPos(ImVec2(6.f + windowPadding.x, 6.0f + titlebarVerticalOffset));

				{
					const ImRect menuBarRect = { ImGui::GetCursorPos(), { ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetFrameHeightWithSpacing() } };

					ImGui::BeginGroup();
					if (UI::BeginMenubar(menuBarRect))
						m_MenubarCallback();

					UI::EndMenubar();
					ImGui::EndGroup();
				}

				if (ImGui::IsItemHovered())
					m_TitlebarHovered = false;

				ImGui::ResumeLayout();
			}

			{
				// Centered Window title
				ImVec2 currentCursorPos = ImGui::GetCursorPos();
				const char* titleName = Rendering::VulkanContext::Window->GetTitle().c_str();
				ImVec2 textSize = ImGui::CalcTextSize(titleName);
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - textSize.x * 0.5f, 2.0f + windowPadding.y + 6.0f));
				ImGui::Text("%s", titleName); // Draw title
				ImGui::SetCursorPos(currentCursorPos);
			}

			// Window buttons
			const ImU32 buttonColN = ColorWithMultipliedValue(Themes::Default::text, 0.9f);
			const ImU32 buttonColH = ColorWithMultipliedValue(Themes::Default::text, 1.2f);
			const ImU32 buttonColP = Themes::Default::textDarker;
			constexpr float buttonSize = 32.f;
			constexpr float iconSize = 12.f;
			const float iconPadding = (buttonSize - iconSize) / 2;

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonSize * 3 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Minimize", ImVec2(buttonSize, buttonSize)))
				Rendering::VulkanContext::Window->Minimize();
			if (ImGui::IsItemHovered())
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), Themes::Default::textDarker);

			const float padY = (buttonSize - (float)m_MinimizeIcon->GetHeight()) / 2.0f;
			UI::DrawButtonImage(*m_MinimizeIcon, buttonColN, buttonColH, buttonColP, RectExpanded(GetItemRect(), -(buttonSize - iconSize) / 2, -padY));


			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonSize * 2 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Maximize", ImVec2(buttonSize, buttonSize))) {
				if (isMaximized)
					Rendering::VulkanContext::Window->Restore();
				else
					Rendering::VulkanContext::Window->Maximize();
			}
			if (ImGui::IsItemHovered())
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), Themes::Default::textDarker);
			UI::DrawButtonImage(isMaximized ? *m_RestoreIcon : *m_MaximizeIcon, buttonColN, buttonColH, buttonColP, RectExpanded(GetItemRect(), -iconPadding, -iconPadding));

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonSize * 1 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Close", ImVec2(buttonSize, buttonSize)))
				Engine::CloseNextFrame();
			if (ImGui::IsItemHovered())
				ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 0, 0, 255));
			UI::DrawButtonImage(*m_CloseIcon, Themes::Default::text, ColorWithMultipliedValue(Themes::Default::text, 1.4f), buttonColP, RectExpanded(GetItemRect(), -iconPadding, -iconPadding));

			ImGui::EndHorizontal();

			ImGui::SetCursorPosY(titlebarHeight);
		}
		else {
			if (ImGui::BeginMainMenuBar()) {
				if (m_MenubarCallback)
					m_MenubarCallback();

				ImGui::EndMainMenuBar();
			}
		}
	}
}