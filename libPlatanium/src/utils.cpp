#include <platanium.hpp>
#include <Windows.h>
#include <TlHelp32.h>

void platanium::epic::utils::kill_anticheats(void)
{
	static std::array<const wchar_t*, 5> kill_process = { xorstr_(L"EpicGamesLauncher.exe"), xorstr_(L"FortniteLauncher.exe"),xorstr_(L"FortniteClient-Win64-Shipping_EAC.exe"), xorstr_(L"FortniteClient-Win64-Shipping_BE.exe"), xorstr_(L"FortniteClient-Win64-Shipping_EAC_EOS.exe")};

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	if (Process32First(hSnapshot, &entry))
	{
		while (Process32Next(hSnapshot, &entry))
		{
			for (auto& process_name : kill_process)
			{
				if (wcscmp(entry.szExeFile, process_name) == 0)
				{
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, entry.th32ProcessID);
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
				}
			}
		}
	}

	CloseHandle(hSnapshot);
}