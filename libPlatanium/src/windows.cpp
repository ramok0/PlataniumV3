#include <platanium.hpp>

NTSTATUS platanium::windows::NtQueryStatusInformation(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
	HMODULE hNtDll = LoadLibrary(xorstr_(L"ntdll.dll"));
	if (!hNtDll) throw std::runtime_error(xorstr_("Failed to load library"));

	PFN_NT_QUERY_SYSTEM_INFORMATION pNtQuerySystemInformation =
		(PFN_NT_QUERY_SYSTEM_INFORMATION)GetProcAddress(hNtDll, xorstr_("NtQuerySystemInformation"));

	if (!pNtQuerySystemInformation) {
		spdlog::error(xorstr_("Failed to find NtQuerySystemInformation function"));
		FreeLibrary(hNtDll);
		throw std::runtime_error(xorstr_("Failed to find NtQuerySystemInformation function"));
	}

	NTSTATUS status = pNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	FreeLibrary(hNtDll);

	return status;
}

NTSTATUS platanium::windows::NtResumeProcess(HANDLE ProcessHandle)
{
    HMODULE hNtDll = LoadLibrary(xorstr_(L"ntdll.dll"));
    if (!hNtDll) throw std::runtime_error(xorstr_("Failed to load library"));

    PFN_NT_RESUME_PROCESS pNtResumeProcess =
        (PFN_NT_RESUME_PROCESS)GetProcAddress(hNtDll, xorstr_("NtResumeProcess"));

    if (!pNtResumeProcess) {
        spdlog::error(xorstr_("Failed to find NtResumeProcess function"));
        FreeLibrary(hNtDll);
        throw std::runtime_error(xorstr_("Failed to find NtResumeProcess function"));
    }

    NTSTATUS status = pNtResumeProcess(ProcessHandle);

    FreeLibrary(hNtDll);

    return status;
}

NTSTATUS platanium::windows::NtSuspendProcess(HANDLE ProcessHandle)
{
    HMODULE hNtDll = LoadLibrary(xorstr_(L"ntdll.dll"));
    if (!hNtDll) throw std::runtime_error(xorstr_("Failed to load library"));

    PFN_NT_SUSPEND_PROCESS pNtSuspendProcess =
        (PFN_NT_SUSPEND_PROCESS)GetProcAddress(hNtDll, xorstr_("NtSuspendProcess"));

    if (!pNtSuspendProcess) {
        spdlog::error(xorstr_("Failed to find NtSuspendProcess function"));
        FreeLibrary(hNtDll);
        throw std::runtime_error(xorstr_("Failed to find NtSuspendProcess function"));
    }

    NTSTATUS status = pNtSuspendProcess(ProcessHandle);

    FreeLibrary(hNtDll);

    return status;
}