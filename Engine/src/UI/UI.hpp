#pragma once

#include "Core/CoreInclude.hpp"
#include "Rendering/Image.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace Sphynx::UI {
	inline ImU32 ColorWithMultipliedValue(const ImColor& color, float multiplier) {
		const ImVec4& colRow = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
	}


	inline static ImRect GetItemRect() {
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	inline static ImRect RectExpanded(const ImRect& rect, float x, float y) {
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	inline static ImRect RectOffset(const ImRect& rect, float x, float y) {
		ImRect result = rect;
		result.Min.x += x;
		result.Min.y += y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	inline static ImRect RectOffset(const ImRect& rect, ImVec2 xy) {
		return RectOffset(rect, xy.x, xy.y);
	}


	bool BeginMenubar(const ImRect& bar);
	void EndMenubar();


	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax);

	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle);

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax);

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle);

	void DrawButtonImage(Rendering::Image& imageNormal, Rendering::Image& imageHovered, Rendering::Image& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed);

	void DrawButtonImage(Rendering::Image& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed);
}