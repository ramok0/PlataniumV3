#include <platanium.hpp>

NTSTATUS platanium::windows::NtQueryStatusInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
	HMODULE hNtDll = LoadLibrary(L"ntdll.dll");
	if (!hNtDll) throw std::runtime_error("Failed to load library");

	PFN_NT_QUERY_SYSTEM_INFORMATION pNtQuerySystemInformation =
		(PFN_NT_QUERY_SYSTEM_INFORMATION)GetProcAddress(hNtDll, "NtQuerySystemInformation");

	if (!pNtQuerySystemInformation) {
		spdlog::error("Failed to find NtQuerySystemInformation function");
		FreeLibrary(hNtDll);
		throw std::runtime_error("Failed to find NtQuerySystemInformation function");
	}

	NTSTATUS status = pNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	FreeLibrary(hNtDll);

	return status;
}

NTSTATUS platanium::windows::NtResumeProcess(HANDLE ProcessHandle)
{
    HMODULE hNtDll = LoadLibrary(L"ntdll.dll");
    if (!hNtDll) throw std::runtime_error("Failed to load library");

    PFN_NT_RESUME_PROCESS pNtResumeProcess =
        (PFN_NT_RESUME_PROCESS)GetProcAddress(hNtDll, "NtResumeProcess");

    if (!pNtResumeProcess) {
        spdlog::error("Failed to find NtResumeProcess function");
        FreeLibrary(hNtDll);
        throw std::runtime_error("Failed to find NtResumeProcess function");
    }

    NTSTATUS status = pNtResumeProcess(ProcessHandle);

    FreeLibrary(hNtDll);

    return status;
}

NTSTATUS platanium::windows::NtSuspendProcess(HANDLE ProcessHandle)
{
    HMODULE hNtDll = LoadLibrary(L"ntdll.dll");
    if (!hNtDll) throw std::runtime_error("Failed to load library");

    PFN_NT_SUSPEND_PROCESS pNtSuspendProcess =
        (PFN_NT_SUSPEND_PROCESS)GetProcAddress(hNtDll, "NtSuspendProcess");

    if (!pNtSuspendProcess) {
        spdlog::error("Failed to find NtSuspendProcess function");
        FreeLibrary(hNtDll);
        throw std::runtime_error("Failed to find NtSuspendProcess function");
    }

    NTSTATUS status = pNtSuspendProcess(ProcessHandle);

    FreeLibrary(hNtDll);

    return status;
}