#include "../include/window.hpp"
#include "../include/imhelper.h"
#include <imgui/imgui.h>
#include <shellapi.h>
#include "../include/renderer.h"

void AuthentificationWindow::render()
{

	const auto io = ImGui::GetIO();

	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Appearing);
	ImGui::Begin(this->get_title_ansi(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	float height = ImGui::GetContentRegionAvail().y;
	ImGui::SetCursorPosY(height / 3);

	bool hasNoAccounts = platanium::accounts.size() == 0;

	ImGui::CenterText("Welcome on PlataniumV3.");

	if (hasNoAccounts || this->bAddNewAccount)
	{
		this->bIsSelectingNewAccount = true;
		ImGui::CenterText("Please enter the following code on epicgames.com/id/activate");
		ImGui::ItemSize(ImVec2(0, 1));

		ImVec2 size = ImGui::CalcTextSize(this->current_code);
		ImGui::Align(size.x + (ImGui::GetStyle().FramePadding.x * 2), true);
		ImGui::BeginDisabled();
		ImGui::InputText("##code", this->current_code, sizeof(this->current_code), ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		if (this->m_Credentials && !this->m_Credentials->is_expired())
		{
			if (!hasNoAccounts)
			{
				if (ImGui::Button("I'll choose another account"))
				{
					this->bAddNewAccount = false;
					this->bIsSelectingNewAccount = false;
				}
			}

			if (ImGui::AlignButton("Login with Epic Games"))
				::ShellExecuteA(0, "open", this->m_Credentials->verification_uri_complete.c_str(), 0, 0, SW_SHOW);


			constexpr const char* remember_me_text = "Remember Me";
			
			ImGui::Align(ImGui::CalcTextSize(remember_me_text).x + (ImGui::GetStyle().FramePadding.x * 2));
			ImGui::Checkbox(remember_me_text, &this->m_RememberMe);
		}
	}
	else {
		ImGui::CenterText("Please choose your account.");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
		for (auto& account : platanium::accounts)
		{
			bool selected = account.first == this->selectedAccount;

			auto auth_manager = platanium::get_auth_manager(account.second.type);
			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
			if (ImGui::Selectable(std::format("{} ({})", account.first, auth_manager->get_name()).c_str(), &selected))
			{
				this->selectedAccount = account.first;
			}
			ImGui::PopStyleVar();
		}

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();


		if (ImGui::AlignButton("I'll use a new account"))
			this->bAddNewAccount = true;

		if (this->selectedAccount.size() != 0 && ImGui::AlignButton(std::format("Use PlataniumV3 as {}", this->selectedAccount).c_str()))
		{
			std::string display_name = this->selectedAccount;
			auto it = std::find_if(platanium::accounts.begin(), platanium::accounts.end(), [display_name](auto& account)
				{
					return account.first == display_name;
				});

			if (it == platanium::accounts.end())
			{
				spdlog::error("failed to find account");
			}
			else {
				if (platanium::login(it->second))
				{
					//this->Close();
					this->bCloseWindow = true;
					this->bIsSelectingNewAccount = false;
					this->m_renderer->SetWindowModel<MainWindow>();
				}
				else {
					std::erase_if(platanium::accounts, [display_name](auto& account)
						{
							return account.first == display_name;
						});

					this->selectedAccount = std::string();

					platanium::configuration->write();
				}
			}
		}
	}

	ImGui::End();
}

void AuthentificationWindow::OnAppear()
{
	std::thread([this]() {
		while (!bCloseWindow && !(m_renderer && this->m_renderer->WindowShouldClose()))
		{
			if (bIsSelectingNewAccount)
			{
				m_Credentials = platanium::epic::api::account::authentfication::create_device_code();

				strcpy_s(this->current_code, m_Credentials->user_code.c_str());

				const auto device_code = m_Credentials->device_code;
				platanium::authentification::Credentials auth_credentials;
				auth_credentials.device_code = m_Credentials->device_code;
				auth_credentials.type = platanium::EPIC_DEVICE_CODE;
				auth_credentials.client_id = platanium::epic::api::auth_clients::fortniteNewSwitchGameClient.first;

				std::this_thread::sleep_for(std::chrono::seconds(5));

				while (!platanium::authentification::account::is_account_valid() && bIsSelectingNewAccount && !bCloseWindow)
				{
					if (m_Credentials->is_expired())
					{
						m_Credentials = platanium::epic::api::account::authentfication::create_device_code();
					}

					if (platanium::login(auth_credentials)) //essaye de se co, ça passe si le mec a validé sur la page
					{
						m_Credentials = std::nullopt;
						if (m_RememberMe)
						{
							platanium::authentification::account::get_current_account()->save();
							platanium::configuration->write();
							this->m_renderer->SetWindowModel<MainWindow>();
						}
					}

					std::this_thread::sleep_for(std::chrono::seconds(10));
				}
			}
		}
		}).detach();
}
