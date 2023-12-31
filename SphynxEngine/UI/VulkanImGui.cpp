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

#include "../../Content/Icons/WindowToolbarIcons.embed"

namespace Sphynx::UI {
	bool s_TitlebarHovered = false;
	std::function<void()> s_MenubarCallback;

	Rendering::Image* s_MinimizeIcon = nullptr;
	Rendering::Image* s_MaximizeIcon = nullptr;
	Rendering::Image* s_RestoreIcon = nullptr;
	Rendering::Image* s_CloseIcon = nullptr;

	float s_MinimizeOpacity = 0.f;
	float s_MaximizeOpacity = 0.f;
	float s_CloseOpacity = 0.f;

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

		ImGui_ImplVulkan_Init(&init_info, Rendering::VulkanContext::Renderpass->GetHandle());

		VkCommandBuffer cmd = Rendering::VulkanContext::CommandPool->BeginSingleUseCommandbuffer();
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		Rendering::VulkanContext::CommandPool->EndSingleUseCommandbuffer(cmd);

		ImGui_ImplVulkan_DestroyFontUploadObjects();

		Themes::SetDefaultTheme();

		Rendering::VulkanContext::Window->SetTitlebarhitTestCallback([]() -> bool {
			return s_TitlebarHovered;
		});


		// Load icons
		s_MinimizeIcon = new Rendering::Image(BufferView(g_WindowMinimizeIcon));
		s_MaximizeIcon = new Rendering::Image(BufferView(g_WindowMaximizeIcon));
		s_RestoreIcon = new Rendering::Image(BufferView(g_WindowRestoreIcon));
		s_CloseIcon = new Rendering::Image(BufferView(g_WindowCloseIcon));
	}

	void VulkanImGui::Shutdown() {
		delete s_MinimizeIcon;
		delete s_MaximizeIcon;
		delete s_RestoreIcon;
		delete s_CloseIcon;

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

			_DrawTitlebar();


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

	void VulkanImGui::SetMenubarCallback(const std::function<void()>& callback) { s_MenubarCallback = callback; }

	ImGuiContext* VulkanImGui::GetContext() {
		return ImGui::GetCurrentContext();
	}

	void VulkanImGui::_DrawTitlebar() {
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

            
			float menubarWidth = 0;

			// Draw Menubar
			if (s_MenubarCallback) {
				ImGui::SetItemAllowOverlap();
				ImGui::SetCursorPos(ImVec2(6.f + windowPadding.x, 6.0f + titlebarVerticalOffset));

				{
					const ImRect menuBarRect = { ImGui::GetCursorPos(), { ImGui::GetContentRegionAvail().x + ImGui::GetCursorScreenPos().x, ImGui::GetFrameHeightWithSpacing() } };

					ImGui::BeginGroup();
					if (UI::BeginMenubar(menuBarRect)) {
						s_MenubarCallback();
						menubarWidth = ImGui::GetCursorPos().x;
					}
					UI::EndMenubar();
					ImGui::EndGroup();
				}

				if (ImGui::IsItemHovered())
					s_TitlebarHovered = false;
			}

			const float w = ImGui::GetContentRegionAvail().x;
			constexpr float buttonWidth = 44.f;
			constexpr float buttonHeight = (buttonWidth * 2) / 3; // width / height = 1.5
			constexpr float buttonsAreaWidth = buttonWidth * 3;

			ImGui::BeginGroup();

			// Title bar drag area
			// On Windows we hook into the GLFW win32 window internals
			ImGui::SetCursorPos(ImVec2(windowPadding.x + menubarWidth, windowPadding.y + titlebarVerticalOffset)); // Reset cursor pos
			// DEBUG DRAG BOUNDS
			//ImGui::GetForegroundDrawList()->AddRect(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + w - (buttonsAreaWidth + menubarWidth), ImGui::GetCursorScreenPos().y + titlebarHeight), IM_COL32(255, 0, 0, 255));
			ImGui::InvisibleButton("##titleBarDragZone", ImVec2(w - (buttonsAreaWidth + menubarWidth), titlebarHeight));

			s_TitlebarHovered = ImGui::IsItemHovered();

			if (isMaximized)
			{
				float windowMousePosY = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
				if (windowMousePosY >= 0.0f && windowMousePosY <= 5.0f)
					s_TitlebarHovered = true; // Account for the top-most pixels which don't register
			}

			{
				// Centered Window title
				ImVec2 currentCursorPos = ImGui::GetCursorPos();
				const char* titleName = Rendering::VulkanContext::Window->GetTitle().c_str();
				ImVec2 textSize = ImGui::CalcTextSize(titleName);
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - textSize.x * 0.5f, 2.0f + windowPadding.y + 6.0f));
				ImGui::TextUnformatted(titleName); // Draw title
				ImGui::SetCursorPos(currentCursorPos);
			}

			// Window buttons
			const ImU32 buttonColN = ColorWithMultipliedValue(Themes::Default::text, 0.9f);
			const ImU32 buttonColH = ColorWithMultipliedValue(Themes::Default::text, 1.2f);
			const ImU32 buttonColP = Themes::Default::textDarker;
			ImColor backgroundH = Themes::Default::textDarker;
			const ImVec2 buttonSize{ buttonWidth, buttonHeight };
			constexpr float iconSize = 12.f;
			const float iconPaddingWidth = (buttonWidth - iconSize) / 2;
			const float iconPaddingHeight = (buttonHeight - iconSize) / 2;
			const float backgroundAnimationDelta = Engine::DeltaTime() / .1f;

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonWidth * 3 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Minimize", buttonSize))
				Rendering::VulkanContext::Window->Minimize();
			if (ImGui::IsItemHovered())
				s_MinimizeOpacity = std::clamp(s_MinimizeOpacity + backgroundAnimationDelta, 0.f, 1.f);
			else
				s_MinimizeOpacity = std::clamp(s_MinimizeOpacity - backgroundAnimationDelta, 0.f, 1.f);
			backgroundH.Value.w = s_MinimizeOpacity; 
			ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), backgroundH);
			
			const float padY = (buttonHeight - (float)s_MinimizeIcon->GetHeight()) / 2.0f;
			UI::DrawButtonImage(*s_MinimizeIcon, buttonColN, buttonColH, buttonColP, RectExpanded(GetItemRect(), -(buttonWidth - iconSize) / 2, -padY));


			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonWidth * 2 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Maximize", buttonSize)) {
				if (isMaximized)
					Rendering::VulkanContext::Window->Restore();
				else
					Rendering::VulkanContext::Window->Maximize();
			}
			if (ImGui::IsItemHovered())
				s_MaximizeOpacity = std::clamp(s_MaximizeOpacity + backgroundAnimationDelta, 0.f, 1.f);
			else
				s_MaximizeOpacity = std::clamp(s_MaximizeOpacity - backgroundAnimationDelta, 0.f, 1.f);
			backgroundH.Value.w = s_MaximizeOpacity;
			ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), backgroundH);
			UI::DrawButtonImage(isMaximized ? *s_RestoreIcon : *s_MaximizeIcon, buttonColN, buttonColH, buttonColP, RectExpanded(GetItemRect(), -iconPaddingWidth, -iconPaddingHeight));

			ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - buttonWidth * 1 - 2.f, windowPadding.y + 2.f));
			if (ImGui::InvisibleButton("Close", buttonSize))
				Engine::CloseNextFrame();
			if (ImGui::IsItemHovered())
				s_CloseOpacity = std::clamp(s_CloseOpacity + backgroundAnimationDelta, 0.f, 1.f);
			else
				s_CloseOpacity = std::clamp(s_CloseOpacity - backgroundAnimationDelta, 0.f, 1.f);
			backgroundH = { 1.f, 0.f, 0.f, s_CloseOpacity };
			ImGui::GetCurrentWindow()->DrawList->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), backgroundH);
			UI::DrawButtonImage(*s_CloseIcon, Themes::Default::text, ColorWithMultipliedValue(Themes::Default::text, 1.4f), buttonColP, RectExpanded(GetItemRect(), -iconPaddingWidth, -iconPaddingHeight));

			ImGui::EndGroup();//ImGui::EndHorizontal();

			ImGui::SetCursorPosY(titlebarHeight);
		}
		else {
			if (ImGui::BeginMainMenuBar()) {
				if (s_MenubarCallback)
					s_MenubarCallback();

				ImGui::EndMainMenuBar();
			}
		}
	}
}