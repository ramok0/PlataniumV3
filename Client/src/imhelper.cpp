#include "imhelper.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

void ImGui::Align(float width, bool set_next_item_with, float alignment)
{
	ImGuiStyle& style = ImGui::GetStyle();
	float avail = ImGui::GetContentRegionAvail().x;
	float off = (avail - width) * alignment;
	if (off > 0.0f)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

		if (set_next_item_with)
			ImGui::SetNextItemWidth(width);
	}
}

bool ImGui::AlignButton(const char* label, ImVec2 size_arg)
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 label_size = ImGui::CalcTextSize(label);
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	ImGui::Align(size.x, false);
	return ImGui::Button(label, size_arg);
}

void ImGui::CenterText(const char* text)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::TextUnformatted(text);
}
void ImGui::CenterTextColored(const char* text, ImVec4 Color)
{
	auto windowWidth = ImGui::GetWindowSize().x;
	auto textWidth = ImGui::CalcTextSize(text).x;

	ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
	ImGui::TextColored(Color, text);
}
