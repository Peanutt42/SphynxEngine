#include "pch.hpp"
#include "UI.hpp"

namespace Sphynx::UI {
	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		auto* drawList = ImGui::GetForegroundDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage(imagePressed.GetDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage(imageHovered.GetDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
		else
			drawList->AddImage(imageNormal.GetDescriptorSet(), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
	};

	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	};

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};


	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}
	
	
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
}