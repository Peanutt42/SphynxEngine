#include "pch.hpp"
#include "UI.hpp"
#include "Profiling/Profiling.hpp"

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

namespace Sphynx::UI {
    bool Vec3(std::string_view label, glm::vec3& v, float resetValue, float columnWidth) {
		bool changed = false;
		ImFont* extraBoldFont = Fonts::Get(Fonts::Type::ExtraBold);

		ImGui::PushID(label.data());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.data());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ .8f, .1f, .15f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ .9f, .2f, .2f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ .8f, .1f, .15f, 1 });
		ImGui::PushFont(extraBoldFont);
		if (ImGui::Button("X", buttonSize)) {
			changed = true;
			v.x = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &v.x, .1f, 0, 0, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ .2f, .7f, .2f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ .3f, .8f, .3f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ .2f, .7f, .2f, 1 });
		ImGui::PushFont(extraBoldFont);
		if (ImGui::Button("Y", buttonSize)) {
			changed = true;
			v.y = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &v.y, .1f, 0, 0, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ .1f, .25f, .8f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ .2f, .35f, .9f, 1 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ .1f, .25f, .8f, 1 });
		ImGui::PushFont(extraBoldFont);
		if (ImGui::Button("Z", buttonSize)) {
			changed = true;
			v.z = resetValue;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &v.z, .1f, 0, 0, "%.2f"))
			changed = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return changed;
	}


	void Init(Rendering::Window& window) {
		SE_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().IniFilename = "imgui.ini";

		Fonts::Init();

		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window.GetGLFWHandle(), true);

		ImGui_ImplOpenGL3_Init("#version 330");
	}

	void Shutdown() {
		SE_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Begin() {
		SE_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
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

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
			static bool opened = true;
			ImGui::Begin("Dockspace", &opened, window_flags);

			ImGui::PopStyleVar(2);


			ImGuiID dockspace_id = ImGui::GetID("Dockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
		}
	}

	void End() {
		SE_PROFILE_FUNCTION();

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
			ImGui::End();

		ImGui::Render();
	}

	void Render() {
		SE_PROFILE_FUNCTION();

		if (auto* drawData = ImGui::GetDrawData())
			ImGui_ImplOpenGL3_RenderDrawData(drawData);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	ImGuiContext* GetContext() {
		return ImGui::GetCurrentContext();
	}
}