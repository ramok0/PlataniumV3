#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ImGui {
	bool AlignButton(const char* label, ImVec2 size_arg = {0.f,0.f});
	void Align(float width, bool set_next_item_with = false, float alignment = 0.5f);
	void CenterText(const char* text);
	void CenterTextColored(const char* text, ImVec4 color);
}