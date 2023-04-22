#include "../include/plataniumv3launcher.hpp"


void gui_render(void)
{

	ImGui::SetNextWindowSize({ WINDOW_WIDTH, WINDOW_HEIGHT }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0.f, 0.f }, ImGuiCond_Appearing);
	ImGui::Begin("##window", 0, WINDOW_FLAGS);

	ImGui::Text("hello world");

	ImGui::End();

	//ImGuiStyle& styles = ImGui::GetStyle();
	//ImGui::ShowStyleEditor(&styles);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
	ImGui::RenderNotifications(); // <-- Here we render all notifications
	ImGui::PopStyleVar(1); // Don't forget to Pop()
	ImGui::PopStyleColor(1);
}