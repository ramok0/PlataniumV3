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

static ImGui::FileBrowser fileDialog = ImGui::FileBrowser(ImGuiFileBrowserFlags_SelectDirectory);

void render_main_form(void)
{
	static char buf1[MAX_PATH];
	static char buf2[MAX_PATH];
	const float inputFloat = 370.f;


	const std::string text = std::format("Hi {}, welcome to PlataniumV3 !\nThis launcher has been made from scratch by github.com/Ramokprout\nInspired by PlataniumV3 by kem0x and Platanium by WorkingRobot", (*current_epic_account)->display_name);

	ImVec2 textSize = ImGui::CalcTextSize(text.c_str());


	//center the content vertically
	float height = ImGui::GetContentRegionAvail().y;
	ImGui::SetCursorPosY(height / 3);

	ImGui::Align(inputFloat);
	ImGui::Text(text.c_str());



	ImGui::Align(inputFloat);
	ImGui::SetNextItemWidth(inputFloat);
	if (!g_configuration->useProxy)
		ImGui::BeginDisabled();
	ImGui::InputText("Detour Proxy", buf2, sizeof(buf2), g_configuration->useProxy ? 0 : ImGuiInputTextFlags_ReadOnly);

	if (!g_configuration->useProxy)
		ImGui::EndDisabled();

	if (!g_configuration->detourURL)
		ImGui::BeginDisabled();

	ImGui::Align(inputFloat);
	ImGui::SetNextItemWidth(inputFloat);
	ImGui::InputText("Forward Host", buf1, sizeof(buf1), g_configuration->detourURL ? 0 : ImGuiInputTextFlags_ReadOnly);
	ImGui::Align(inputFloat);
	ImGui::SetNextItemWidth(inputFloat);
	ImGui::InputInt("Forward Port", &g_configuration->forwardPort, 1, 100, g_configuration->detourURL ? 0 : ImGuiInputTextFlags_ReadOnly);

	if (!g_configuration->detourURL)
		ImGui::EndDisabled();

	ImGuiStyle& style = ImGui::GetStyle();
	float avail = ImGui::GetContentRegionAvail().x;
	float off = (avail - inputFloat) * 0.5;

	float originalPosX = ImGui::GetCursorPosX();

	ImVec2 boutonSize = { (inputFloat - (style.ItemSpacing.x * 2)) / 3, 25.f };

	ImGui::SetCursorPosX(originalPosX + off);
	ImGui::Checkbox("Detour URL", &g_configuration->detourURL);
	ImGui::SameLine();
	ImGui::SetCursorPosX(originalPosX + off + boutonSize.x + style.ItemSpacing.x);
	ImGui::Checkbox("Disable SSL", &g_configuration->disableSSL);
	ImGui::SameLine();
	ImGui::SetCursorPosX(originalPosX + off + (boutonSize.x + style.ItemSpacing.x) * 2);
	ImGui::Checkbox("Use Proxy", &g_configuration->useProxy);	
	
	ImGui::SetCursorPosX(originalPosX + off);
	ImGui::Checkbox("Dump AES (experimental)", &g_configuration->useProxy);

	ImGui::SetCursorPosX(originalPosX + off);
	ImGui::Checkbox("Disable Signature Checks (experimental)", &g_configuration->should_check_pak);





	ImGui::Align(boutonSize.x * 3 + ImGui::GetStyle().ItemSpacing.x * 2);

	if (ImGui::Button("Start Fortnite", boutonSize))
	{
		std::thread(start_fortnite_and_inject_dll).detach();
	}
	ImGui::SameLine();
	if (ImGui::Button("Edit Fortnite Path", boutonSize))
	{
		fileDialog.Open();
	}
	ImGui::SameLine();
	if (ImGui::Button("Save settings", boutonSize))
	{
		write_configuration();
		ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Applied configuration successfully" });
	}
}

void render_epic_games_login_form(void)
{
	static bool rememberMe = true;
	const float inputTextWidth = 300.f;
	static const char* text = "Authorization Code";
	static char buf[33];

	ImGuiStyle& style = ImGui::GetStyle();

	//center the content vertically
	float height = ImGui::GetContentRegionAvail().y;
	ImGui::SetCursorPosY(height / 3);

	//center the "authorization code" text horizontally
	ImGui::Align(ImGui::CalcTextSize(text).x);
	ImGui::Text(text);

	//center the text input horizontally
	ImGui::SetNextItemWidth(inputTextWidth);
	ImGui::Align(inputTextWidth);
	ImGui::InputText("##code", buf, sizeof(buf));

	//calculate the size of the button for it to be centered with the inputtext
	ImVec2 buttonSize = { (inputTextWidth / 2) - (ImGui::GetStyle().ItemSpacing.x / 2), 25.f };

	//align the button horizontally
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
			ImGui::InsertNotification({ ImGuiToastType_Success, 3000, std::format("Connected as {}", account_buffer->display_name).c_str() });
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
		//create a web browser instance to the EPIC_GENERATE_AUTHORIZATION_CODE_URL constant
		ShellExecuteA(GetDesktopWindow(), "open", std::format(EPIC_GENERATE_AUTHORIZATION_CODE_URL, FORTNITE_IOS_GAME_CLIENT_ID).c_str(), "", "", SW_SHOW);
	}
	//align the checkbox with the inputtext
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
		render_main_form();
	}

	ImGui::End();

	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		if (!verify_fortnite_directory(fileDialog.GetSelected()))
		{
			ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Invalid Fortnite folder selected. Please make sure that your Fortnite directory contains a 'Engine' and a 'FortniteGame' directory inside it." });
			spdlog::warn("Invalid Fortnite folder selected.");
		}
		else {
			g_configuration->fortnite_path = fileDialog.GetSelected().string();
			write_configuration();
			ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Updated Fortnite directory successfully" });
		}
		fileDialog.ClearSelected();
	}

	/*ImGuiStyle& styles = ImGui::GetStyle();
	ImGui::ShowStyleEditor(&styles);*/
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
	ImGui::RenderNotifications(); // <-- Here we render all notifications
	ImGui::PopStyleVar(1); // Don't forget to Pop()
	ImGui::PopStyleColor(1);
}