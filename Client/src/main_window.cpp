#include "../include/window.hpp"
#include "../include/imhelper.h"
#include <imgui/imgui.h>
#include <iostream>

void MainWindow::render()
{
	static platanium::epic::Fortnite* fHandle;
	static platanium::epic::FortniteHandle* gHandle;
	static std::vector<platanium::epic::FortniteHandle*> gHandles;

	if(!fHandle)
		fHandle = new platanium::epic::Fortnite(platanium::configuration->get()->fortnite_path);

	const auto io = ImGui::GetIO();

	ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Appearing);
	ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Appearing);
	ImGui::Begin(this->get_title_ansi(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	std::string sText = std::format("Welcome back, {}", platanium::authentification::account::get_current_account()->get_details().display_name.c_str());

	ImGui::CenterText(sText.c_str());
	ImGui::CenterText("This version is an unfinished version of PlataniumV3.");
	ImGui::CenterTextColored("If you encounter any problem, create an issue on github.", ImColor(255, 0, 0).Value);
	
	if (ImGui::AlignButton("Start Fortnite"))
	{
		if (!(gHandle = fHandle->start(fHandle->get_start_arguments(
			platanium::authentification::account::get_current_account()->get_authentification_arguments()
		))))
		{
			spdlog::debug("failed to start fortnite");
		}

		if (gHandle)
		{
			gHandle->suspend();
			if (gHandle->inject(std::filesystem::current_path() / "Platanium.dll"))
			{
				spdlog::info("DLL has been injected.");
			}

			if (gHandle->resume())
			{
				spdlog::info("resumed app");
			}
			else {
				spdlog::warn("Failed to resume thread");
			}
		}
	}

	if (gHandle && gHandle->is_alive())
	{
		ImGui::TextUnformatted("Fortnite Is Alive.");
	}

	ImGui::End();
}

void MainWindow::OnAppear()
{

}
