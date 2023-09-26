#pragma once
#include <platanium.hpp>
#include "ue4.h"

namespace patterns {
	constexpr const char* CURL_EASY_SETOPT = "89 54 24 10 4C 89 44 24 ? 4C 89 4C 24 ? 48 83 EC 28 48 85 C9";
	constexpr const wchar_t* GET_ENGINE_VERSION = L"unreal-v%i-%s.dmp";
}

class Platanium {
public:
	Platanium(void* hModule)
	{

		this->m_baseAddress = hModule;

		if (!platanium::initialize(platanium::CLIENT_TYPE::PLATANIUM_CLIENT))
		{
			MessageBoxA(0, "Failed to initialize libPlatanium !\nPlease make sure you are using a Platanium-compliant launcher", "PlataniumV3", MB_OK);
		}

		this->curl_easy_setopt = 0;
		this->curl_setopt = 0;
		this->get_engine_version = 0;
	}

	~Platanium() {
	//	MH_Uninitialize();
	//	FreeConsole();
		platanium::destroy();
	}

	FEngineVersion* GetEngineVersion( void );

	void find_patterns( void );

	void place_hooks( void );

	static CURLcode __fastcall hkCurlEasySetopt(void* curl, CURLoption opt, ...);

	CURLcode curl_setopt_w(void* handle, CURLoption opt, ...);

private:
	void* m_baseAddress;
	void* (*get_engine_version)();
	CURLcode(*curl_easy_setopt)(void*, CURLoption, ...);
//	CURLcode(*oCurl_easy_setopt)(void*, CURLoption, ...);
	CURLcode(__fastcall* curl_setopt)(void*, int, va_list);
};

inline std::unique_ptr<Platanium> g_application;