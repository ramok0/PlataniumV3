#include "../include/plataniumv3launcher.hpp"

namespace ImGui {
	void Align(float width, float alignment = 0.5f)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		float avail = ImGui::GetContentRegionAvail().x;
		float off = (avail - width) * alignment;
		if (off > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
	}
}


void render_epic_games_login_form(void)
{
	static bool rememberMe = true;
	static const char* text = "Authorization Code";
	static char buf[33];

	ImGuiStyle& style = ImGui::GetStyle();

	float height = ImGui::GetContentRegionAvail().y;
	ImGui::SetCursorPosY(height / 3);

	ImGui::Align(ImGui::CalcTextSize(text).x);
	ImGui::Text(text);
	const float inputTextWidth = 300.f;
	ImGui::SetNextItemWidth(inputTextWidth);
	ImGui::Align(inputTextWidth);
	ImGui::InputText("##code", buf, sizeof(buf));

	ImVec2 buttonSize = { 150.f - (ImGui::GetStyle().ItemSpacing.x / 2), 25.f };

	ImGui::Align(ImGui::GetStyle().ItemSpacing.x + (buttonSize.x * 2));

	if (ImGui::Button("Login", buttonSize))
	{
		std::string authorizationCode = std::string(buf);

		epic_account_t* account_buffer = new epic_account_t();

		if (epic_login_with_authorization_code(authorizationCode, account_buffer))
		{
			epic_device_auth_t deviceAuth;
			if (rememberMe)
			{
				if (epic_create_device_auth(&deviceAuth))
				{
					g_configuration->deviceAuth = deviceAuth;
					write_configuration();
				}
				else {
					ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to create device_auth, 'remember be' will not work !" });
				}
			}
			ImGui::InsertNotification({ ImGuiToastType_Success, 3000, std::format("Connected as {}", account_buffer->display_name).c_str()});
		}
		else {
			memset(buf, 0, sizeof(buf));
			ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to login to epic games services !" });
			delete account_buffer;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Get Code", buttonSize))
	{
		ShellExecuteA(GetDesktopWindow(), "open", std::format(EPIC_GENERATE_AUTHORIZATION_CODE_URL, FORTNITE_IOS_GAME_CLIENT_ID).c_str(), "", "", SW_SHOW);
	}

	ImGui::Align(ImGui::GetStyle().ItemSpacing.x + (buttonSize.x * 2));
	ImGui::Checkbox("Remember me", &rememberMe);
}

void gui_render(void)
{

	ImGui::SetNextWindowSize({ WINDOW_WIDTH, WINDOW_HEIGHT }, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0.f, 0.f }, ImGuiCond_Appearing);
	ImGui::Begin(get_window_title().c_str(), 0, WINDOW_FLAGS);

	if (*current_epic_account == nullptr)
	{
		render_epic_games_login_form();
	}
	else {
		ImGui::Text("hello world");
	}

	ImGui::End();

	ImGuiStyle& styles = ImGui::GetStyle();
	ImGui::ShowStyleEditor(&styles);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
	ImGui::RenderNotifications(); // <-- Here we render all notifications
	ImGui::PopStyleVar(1); // Don't forget to Pop()
	ImGui::PopStyleColor(1);
}