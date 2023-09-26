#include <Windows.h>
#include "../include/platanium.h"
#include <xorstr.hpp>
#include <iostream>

void Main( void* hModule )
{
    spdlog::set_level(spdlog::level::level_enum::trace);

    g_application = std::make_unique<Platanium>(new Platanium(hModule));
    g_application->find_patterns();
    g_application->place_hooks();
    
    const auto version = g_application->GetEngineVersion();
    
    spdlog::debug("Found Unreal Engine Version : {}.{}.{}", version->Major, version->Minor, version->Patch);
}

__declspec(dllexport) bool __stdcall DllMain( void* hModule,
                       unsigned long  reason,
                       void* lpReserved
                     )
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        Main(hModule);
     //   CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, nullptr);
    }

    return true;
}

