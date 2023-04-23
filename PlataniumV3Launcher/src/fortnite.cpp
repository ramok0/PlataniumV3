#include "../include/plataniumv3launcher.hpp"
#include <processthreadsapi.h>

bool fortnite_find_default_installation_path(fs::path& fortnite_out_path)
{
	fs::path launcherInstalled = fs::path(EPIC_LAUNCHER_INSTALLED_PATH);

	if (!fs::exists(launcherInstalled)) return false;

	std::ifstream stream(launcherInstalled);

	if (!stream.is_open())
	{
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(stream);
	
	if (data.find("InstallationList") == data.end())
	{
		stream.close();
		return false;
	}

	for (auto& installation : data["InstallationList"])
	{
		if (installation.find("ItemId") == installation.end()) continue;
		std::string itemId = installation["ItemId"].get<std::string>();
		if (itemId != FORTNITE_ITEM_ID) continue;

		if (installation.find("InstallLocation") == installation.end()) break;

		std::string installLocation = installation["InstallLocation"].get<std::string>();
		spdlog::info("Found Fortnite install location => {}", installLocation);
		fortnite_out_path = fs::path(installLocation);
		return true;
	}
	

	stream.close();
	return false;
}

std::string generate_fortnite_start_arguments(fs::path fortnitePath, fs::path configPath, std::string exchangeCode)
{
	return std::format("\"{}\" -EpicPortal -AUTH_LOGIN=unused -AUTH_PASSWORD={} -AUTH_TYPE=exchangecode -epicapp=Fortnite -epicenv=Prod -epicusername={} -epicuserid={} -nobe -noeac -plataniumconfigpath={}", fortnitePath.string(), exchangeCode, (*current_epic_account)->display_name, (*current_epic_account)->account_id, configPath.string());
}

bool start_fortnite_and_inject_dll(void)
{
	if (!verify_fortnite_directory(g_configuration->fortnite_path))
	{
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Your Fortnite path is invalid !" });
		spdlog::error("Invalid Fortnite Path, cannot start Fortnite.");
		return false;
	}

	fs::path fortniteBinary = fs::path(g_configuration->fortnite_path) / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe";

	fs::path dllPath = fs::current_path() / "PlataniumV3.dll";

	if (!fs::exists(dllPath))
	{
		spdlog::error("Failed to find PlataniumV3.dll at {}", dllPath.string());
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to find PlataniumV3.dll" });
		return false;
	}

	std::string exchangeCode;
	if (!epic_create_exchange_code(exchangeCode))
	{
		delete* current_epic_account;
		*current_epic_account = nullptr;
		if (!g_configuration->deviceAuth.account_id.empty() && !g_configuration->deviceAuth.device_id.empty() && !g_configuration->deviceAuth.secret.empty())
		{
			epic_account_t* user = new epic_account_t();
			if (!epic_login_with_device_auth(g_configuration->deviceAuth, user))
			{
				spdlog::error("Failed to re-auth");
				ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to re-auth using deviceauth." });
			}
		}

		spdlog::error("Failed to create exchange code");
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to create exchange code" });
		return false;
	}

	fs::path configPath = fs::current_path() / "config.json";

	if (!fs::exists(configPath))
	{
		spdlog::error("Failed to find configuration file");
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to find configuration" });
		return false;
	}

	LPVOID LoadLibraryAddress = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	STARTUPINFOA startupInfo{0};
	PROCESS_INFORMATION processInfo{0};
	std::string arguments = generate_fortnite_start_arguments(fortniteBinary, configPath, exchangeCode);
	if (!CreateProcessA(nullptr, (LPSTR)arguments.c_str(), nullptr, nullptr, false, 0, nullptr, fortniteBinary.parent_path().string().c_str(), &startupInfo, &processInfo))
	{
		spdlog::error("Failed to create fortnite process, error : {}", GetLastError());
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to start fortnte" });
		return false;
	}

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	//std::this_thread::sleep_for(std::chrono::seconds(4));

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processInfo.dwProcessId);

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		spdlog::error("Failed to open handle");
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		return false;
	}

	LPVOID buffer = VirtualAllocEx(hProcess, nullptr, dllPath.string().size(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!buffer)
	{
		spdlog::error("VirtualAllocEx failed with error code: {}", GetLastError());
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return false;
	}

	std::string path = dllPath.string();

	SIZE_T NumberOfBytesWritten;
	if (!WriteProcessMemory(hProcess, buffer, path.c_str(), path.size(), &NumberOfBytesWritten))
	{
		spdlog::error("WriteProcessMemory failed with error code: {}", GetLastError());
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return false;
	}

	HANDLE hLoadThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddress, buffer, 0, 0);

	if (hLoadThread == INVALID_HANDLE_VALUE)
	{
		spdlog::error("CreateRemoteThread failed with error code: {}", GetLastError());
		ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return false;
	}

	CloseHandle(hLoadThread);

	spdlog::info("Started Fortnite and injected DLL successfully, fortnite PID: {}", processInfo.dwProcessId);
	ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Started Fortnite and Injected DLL successfully !" });

	return true;
}

bool verify_fortnite_directory(fs::path directory)
{
	if (!fs::exists(directory / "FortniteGame") && fs::exists(directory / "Engine")) {
		return false;
	}

	return fs::exists(directory / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe");
}