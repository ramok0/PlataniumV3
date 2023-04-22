#include "../include/platanium.hpp"

void log_pointer(const char* pointerName, void* address, bool showOffset)
{
	std::cout << std::format("[plataniumv3] - [{}] = {}", pointerName, address, showOffset) << (showOffset ? std::format(" ({})", (LPVOID)((uintptr_t)address - (uintptr_t)GetModuleHandle(0))) : "") << std::endl;
}