#include <platanium.hpp>
#include <tlhelp32.h>
#include <ntstatus.h>
#include <iostream>

typedef enum _THREAD_STATE {
	Running = 0,
	Waiting = 1,
	Transition = 2,
	DeferredReady = 3,
	GateWaitObsolete = 4,
	Blocked = 5
} THREAD_STATE;

bool platanium::epic::FortniteHandle::is_alive(void)
{
	DWORD exitCode;

	return GetExitCodeProcess(this->m_process_info.hProcess, &exitCode) > 0 && exitCode == STILL_ACTIVE;
}

bool platanium::epic::FortniteHandle::is_suspended(void)
{
	ULONG returnLength;
	NTSTATUS status;
	status = windows::NtQueryStatusInformation(SystemProcessInformation, nullptr, 0, &returnLength);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
	{
		spdlog::error("NtQuerySystemInformation failed (1)");
		return false;
	}
	
	/*
	i have to add this sketchy code because when we resume fortnite process, fortnite will create threads and returnLength will be too small for calling NtQueryStatusInformation
	as a result, i have to allocate more memory than expected
	*/
	returnLength += 1000; 

	PVOID buffer = ::operator new(returnLength);
	if (!buffer) {
		spdlog::error("Failed to allocate memory");
		return false;
	}

	status = windows::NtQueryStatusInformation(SystemProcessInformation, buffer, returnLength, &returnLength);
	if (status != 0) {
		spdlog::error("NtQuerySystemInformation failed (2) lasterror: {}", GetLastError());
		std::cout << std::hex << status << std::endl;
		::operator delete(buffer);
		return 1;
	}

	PSYSTEM_PROCESS_INFORMATION pProcessInfo = reinterpret_cast<PSYSTEM_PROCESS_INFORMATION>(buffer);

	while (pProcessInfo->NextEntryOffset != 0) {
		if ((DWORD)pProcessInfo->UniqueProcessId == this->get_process_info()->dwProcessId)
		{
			PSYSTEM_THREAD_INFORMATION Threads = (PSYSTEM_THREAD_INFORMATION)(((BYTE*)pProcessInfo) + sizeof(SYSTEM_PROCESS_INFORMATION));
			for (unsigned int i = 0; i < pProcessInfo->NumberOfThreads; i++)
			{
				if (Threads[i].ThreadState != THREAD_STATE::Blocked)
				{
					::operator delete(buffer);
					return false;
				}
			}
		}

		pProcessInfo = reinterpret_cast<PSYSTEM_PROCESS_INFORMATION>(
			reinterpret_cast<PUCHAR>(pProcessInfo) + pProcessInfo->NextEntryOffset);
	}

	::operator delete(buffer);
	return true;
}

bool platanium::epic::FortniteHandle::suspend(void)
{
	if (this->is_suspended()) return true;

	return NT_SUCCESS(windows::NtSuspendProcess(this->get_process_info()->hProcess));
}

bool platanium::epic::FortniteHandle::resume(void)
{
	spdlog::debug("trying to resume app..");
	return NT_SUCCESS(windows::NtResumeProcess(this->get_process_info()->hProcess));
}

bool platanium::epic::FortniteHandle::inject(std::filesystem::path dll_path)
{
	if (!this->is_alive()) return false;

	std::string dll_path_string = dll_path.string();
	size_t dll_path_size = dll_path_string.size();

	const LPVOID lpPathAddress = VirtualAllocEx(this->get_process_info()->hProcess, nullptr, dll_path_size + 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (lpPathAddress == nullptr) return false;

	const DWORD dwWriteResult = WriteProcessMemory(this->get_process_info()->hProcess, lpPathAddress, dll_path_string.c_str(), dll_path_size + 1, nullptr);
	if (dwWriteResult == 0)
	{
		spdlog::error("Failed to write DLL Path");
		//TODO : free memory
		return false;
	}

	const HMODULE hModule = GetModuleHandleA("kernel32.dll");
	const FARPROC lpFunctionAddress = GetProcAddress(hModule, "LoadLibraryA");

	const HANDLE hThreadCreationResult = CreateRemoteThread(this->get_process_info()->hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)lpFunctionAddress, lpPathAddress, 0, nullptr);
	if (hThreadCreationResult == INVALID_HANDLE_VALUE)
	{
		spdlog::error("Failed to create Thread");
		return false;
	}

	VirtualFreeEx(this->get_process_info()->hProcess, lpPathAddress, dll_path_size + 1, MEM_RELEASE);

	spdlog::info("thread has been created");

	return true;
}
