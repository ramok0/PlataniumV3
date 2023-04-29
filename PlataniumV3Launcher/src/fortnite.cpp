#include "../include/plataniumv3launcher.hpp"
#include <processthreadsapi.h>

#pragma comment(lib, "version.lib")

PLATANIUM_FAILURE_REASON fortnite_find_default_installation_path(fs::path& fortnite_out_path)
{
	fs::path launcherInstalled = fs::path(EPIC_LAUNCHER_INSTALLED_PATH);

	if (!fs::exists(launcherInstalled)) return PLATANIUM_FILE_DOES_NOT_EXISTS;

	std::ifstream stream(launcherInstalled);

	if (!stream.is_open())
	{
		return PLATANIUM_FILE_DOES_NOT_EXISTS;
	}

	nlohmann::json data = nlohmann::json::parse(stream);

	if (data.find("InstallationList") == data.end())
	{
		stream.close();
		return PLATANIUM_JSON_MISSING_KEY;
	}

	for (auto& installation : data["InstallationList"])
	{
		if (installation.find("ItemId") == installation.end()) continue;
		std::string itemId = installation["ItemId"].get<std::string>();
		if (itemId != FORTNITE_ITEM_ID) continue;

		if (installation.find("InstallLocation") == installation.end()) break;

		std::string installLocation = installation["InstallLocation"].get<std::string>();
		spdlog::info("{} - Found Fortnite install location => {}", __FUNCTION__, installLocation);
		fortnite_out_path = fs::path(installLocation);
		return PLATANIUM_NO_FAILURE;
	}


	stream.close();
	return PLATANIUM_FAILED_TO_PARSE;
}

std::string generate_fortnite_start_arguments(fs::path fortnitePath, fs::path configPath, std::string exchangeCode)
{
	return std::format("\"{}\" -EpicPortal -AUTH_LOGIN=unused -AUTH_PASSWORD={} -AUTH_TYPE=exchangecode -epicapp=Fortnite -epicenv=Prod -epicusername={} -epicuserid={} -nobe -noeac -plataniumconfigpath=\"{}\"", fortnitePath.string(), exchangeCode, (*current_epic_account)->display_name, (*current_epic_account)->account_id, configPath.string());
}

PLATANIUM_FAILURE_REASON start_fortnite_and_inject_dll(void)
{
	if (!verify_fortnite_directory(g_configuration->fortnite_build.path))
	{
		//ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Your Fortnite path is invalid !" });
		spdlog::error("{} - Invalid Fortnite Path, cannot start Fortnite.", __FUNCTION__);
		return PLATANIUM_FILE_DOES_NOT_EXISTS;
	}

	fs::path fortniteBinary = fs::path(g_configuration->fortnite_build.path) / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe";

	fs::path dllPath = fs::current_path() / "PlataniumV3.dll";

	if (!fs::exists(dllPath))
	{
		spdlog::error("Failed to find PlataniumV3.dll at {}", dllPath.string());
	//	ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to find PlataniumV3.dll" });
		return PLATANIUM_FILE_DOES_NOT_EXISTS;
	}

	std::string exchangeCode;

	PLATANIUM_FAILURE_REASON epic_create_exchange_code_failure_reason = epic_create_exchange_code(exchangeCode);

	if (platalog_error(epic_create_exchange_code_failure_reason, "epic_create_exchange_code") != PLATANIUM_NO_FAILURE) //if the token is expired
	{
		if (PLATANIUM_OK(platalog_error(epic_login_with_refresh_token(), "epic_login_with_refresh_token"))) //if the token is expired and we succesfully refresh it 
		{
			if (PLATANIUM_OK(platalog_error(epic_create_exchange_code(exchangeCode), "epic_create_exchange_code"))) //if the token is expired and we succesfully refresh it and we successfullly create an exchange code with the new exchange code
			{
				spdlog::warn("{} - Invalid token, but refreshed it so its ok", __FUNCTION__);
			}
			else {
				spdlog::error("{} - Refreshed token correctly, but failed to create exchange code still !", __FUNCTION__); //if the token is expired and we succesfully refresh it and we dont manage to create an exchange code with the new exchange code
				return PLATANIUM_FAILED_TO_CREATE_EXCHANGE_CODE;
			}
		}
		else {
			spdlog::warn("{} - Failed to refresh token !", __FUNCTION__); //if the token is invalid and we cant refresh it
			return PLATANIUM_FAILED_TO_REFRESH_TOKEN;
		}
	}

	fs::path configPath = fs::current_path() / "config.json";

	if (!fs::exists(configPath))
	{
		spdlog::error("{} - Failed to find configuration file", __FUNCTION__);
	//	ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to find configuration" });
		return PLATANIUM_FILE_DOES_NOT_EXISTS;
	}

	LPVOID LoadLibraryAddress = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	STARTUPINFOA startupInfo{ 0 };
	PROCESS_INFORMATION processInfo{ 0 };
	std::string arguments = generate_fortnite_start_arguments(fortniteBinary, configPath, exchangeCode);
	if (!CreateProcessA(nullptr, (LPSTR)arguments.c_str(), nullptr, nullptr, false, 0, nullptr, fortniteBinary.parent_path().string().c_str(), &startupInfo, &processInfo))
	{
		spdlog::error("{} - Failed to create fortnite process, error : {}", __FUNCTION__,GetLastError());
		return PLATANIUM_OS_ERROR;
	}

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processInfo.dwProcessId);

	if (hProcess == INVALID_HANDLE_VALUE)
	{
		spdlog::error("{} - Failed to open handle", __FUNCTION__);
		return PLATANIUM_OS_ERROR;
	}

	LPVOID buffer = VirtualAllocEx(hProcess, nullptr, dllPath.string().size(), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!buffer)
	{
		spdlog::error("{} - VirtualAllocEx failed with error code: {}", __FUNCTION__, GetLastError());
	//	ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return PLATANIUM_OS_ERROR;
	}

	std::string path = dllPath.string();

	SIZE_T NumberOfBytesWritten;
	if (!WriteProcessMemory(hProcess, buffer, path.c_str(), path.size(), &NumberOfBytesWritten))
	{
		spdlog::error("{} - WriteProcessMemory failed with error code: {}", __FUNCTION__, GetLastError());
	//	ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return PLATANIUM_OS_ERROR;
	}

	HANDLE hLoadThread = CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryAddress, buffer, 0, 0);

	if (hLoadThread == INVALID_HANDLE_VALUE || hLoadThread == 0)
	{
		spdlog::error("{} - CreateRemoteThread failed with error code: {}", __FUNCTION__, GetLastError());
	//	ImGui::InsertNotification({ ImGuiToastType_Error, 3000, "Failed to inject DLL" });
		CloseHandle(hProcess);
		return PLATANIUM_OS_ERROR;
	}

	CloseHandle(hLoadThread);

	spdlog::info("{} - Started Fortnite and injected DLL successfully, fortnite PID: {}", __FUNCTION__, processInfo.dwProcessId);
//	ImGui::InsertNotification({ ImGuiToastType_Success, 3000, "Started Fortnite and Injected DLL successfully !" });

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON verify_fortnite_directory(fs::path directory)
{
	return fs::exists(directory / "FortniteGame") && fs::exists(directory / "Engine") && fs::exists(directory / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe") ? PLATANIUM_NO_FAILURE : PLATANIUM_FILE_DOES_NOT_EXISTS;
}

PLATANIUM_FAILURE_REASON find_fortnite_engine_version(void)
{
	DWORD dwHandle = 0;

	if (!PLATANIUM_OK(platalog_error(verify_fortnite_directory(g_configuration->fortnite_build.path), "verify_fortnite_directory"))) return PLATANIUM_FILE_DOES_NOT_EXISTS;

	fs::path fortnite_path = fs::path(g_configuration->fortnite_build.path) / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe";

	spdlog::trace("{} - Path: {}", __FUNCTION__, fortnite_path.string().c_str());

	DWORD dwSize = GetFileVersionInfoSizeA(fortnite_path.string().c_str(), &dwHandle);
	if (dwSize == NULL)
	{
		spdlog::error("{} - Error: GetFileVersionInfoSizeA failed with error code : {}", __FUNCTION__, GetLastError());
		return PLATANIUM_OS_ERROR;
	}

	std::vector<BYTE> buffer(dwSize);
	if (!GetFileVersionInfoA(fortnite_path.string().c_str(), NULL, dwSize, buffer.data()))
	{
		spdlog::error("{} - Error: GetFileVersionInfoA failed with error code : {}", __FUNCTION__, GetLastError());
		return PLATANIUM_OS_ERROR;
	}

	VS_FIXEDFILEINFO* pFileInfo;
	UINT uLen;
	if (!VerQueryValueA(buffer.data(), "\\", (LPVOID*)&pFileInfo, &uLen))
	{
		spdlog::error("{} - Error: VerQueryValueA failed with error code : {}", __FUNCTION__, GetLastError());
		return PLATANIUM_OS_ERROR;
	}

	spdlog::trace("Fortnite file version: {}.{}.{}.{}", HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS), LOWORD(pFileInfo->dwFileVersionLS));

	DWORD firstPart = HIWORD(pFileInfo->dwFileVersionMS);
	DWORD secondPart = LOWORD(pFileInfo->dwFileVersionMS);

	std::string strVersion = std::format("{}.{}", firstPart, secondPart); //ceci est égal à "4.20"

	g_configuration->fortnite_build.engine_version = std::stof(strVersion); //ceci est égal a 4.2

	spdlog::debug("{} - found engine version : {}", __FUNCTION__, g_configuration->fortnite_build.engine_version);

	return PLATANIUM_NO_FAILURE;
}