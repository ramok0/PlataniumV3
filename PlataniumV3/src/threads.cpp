#include "../include/platanium.hpp"

void freeze_all_threads(void)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	
	THREADENTRY32 entry;
	entry.dwSize = sizeof(entry);

	DWORD currentThreadId = GetCurrentThreadId();
	DWORD currentProcessId = GetCurrentProcessId();

	if (Thread32First(snapshot, &entry))
	{
		while (Thread32Next(snapshot, &entry))
		{
			if (entry.th32ThreadID == currentThreadId || entry.th32OwnerProcessID != currentProcessId) continue;
			HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, false, entry.th32ThreadID);
			SuspendThread(threadHandle);
			CloseHandle(threadHandle);
		}
	}

	CloseHandle(snapshot);
}

void resume_all_threads(void)
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	THREADENTRY32 entry;
	entry.dwSize = sizeof(entry);

	DWORD currentThreadId = GetCurrentThreadId();
	DWORD currentProcessId = GetCurrentProcessId();

	if (Thread32First(snapshot, &entry))
	{
		while (Thread32Next(snapshot, &entry))
		{
			if (entry.th32ThreadID == currentThreadId || entry.th32OwnerProcessID != currentProcessId) continue;
			HANDLE threadHandle = OpenThread(THREAD_ALL_ACCESS, false, entry.th32ThreadID);
			ResumeThread(threadHandle);
			CloseHandle(threadHandle);
		}
	}

	CloseHandle(snapshot);
}