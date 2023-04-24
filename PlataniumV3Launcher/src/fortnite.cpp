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
		bool tokenRefreshed = epic_login_with_refresh_token();
		if (tokenRefreshed)
		{
			if (epic_create_exchange_code(exchangeCode))
			{
				spdlog::warn("Invalid token, but refreshed it so its ok");
				ImGui::InsertNotification({ ImGuiToastType_Warning, 3000, "Your token was invalid" });
			}
			else {
				spdlog::error("Refreshed token correctly, but failed to create exchange code still !");
				ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to refresh token !" });
				return false;
			}
		}
		else {
			spdlog::warn("Failed to refresh token !");
			ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to refresh token !" });
			return false;
		}
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

	if (hLoadThread == INVALID_HANDLE_VALUE || hLoadThread == 0)
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