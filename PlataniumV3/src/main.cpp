#include "../include/platanium.hpp"

DWORD WINAPI Main(HMODULE hModule)
{
	console* con = new console();

	std::cout << "Welcome to PlataniumV3, fully made by Ramok !" << std::endl;

	std::filesystem::path config_path;

	if (!get_config_path(config_path))
	{
		MessageBoxA(0, "get_config_path returned false", "PlataniumV3", 0);
		return 1;
	}

	read_config(config_path);
	log_pointer("baseaddress", (LPVOID)GetModuleHandle(0));
	freeze_all_threads();
	find_patterns();
	assign_natives();
	check_version();
	place_hooks();

	resume_all_threads();

	//delete con;
//	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  reason, LPVOID lpReserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, nullptr);
	}
	return TRUE;
}

