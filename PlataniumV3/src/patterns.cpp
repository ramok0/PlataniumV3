#include "../include/platanium.hpp"

#define WILDCARD UINT8_MAX
#define assign(dest, address) dest = reinterpret_cast<decltype(dest)>(address);

void find_patterns(void)
{
	Memcury::Scanner curl_easy_setopt = Memcury::Scanner::FindPattern(CURL_EASY_SETOPT);
	addresses::curl_easy_setopt = curl_easy_setopt.GetAs<void*>();
	addresses::curl_setopt = curl_easy_setopt.ScanFor({ Memcury::ASM::CALL }).RelativeOffset(1).GetAs<void*>();
	//this is by far the best way to find this function
	addresses::request_exit_with_status = Memcury::Scanner::FindStringRef(REQUEST_EXIT_WITH_STATUS).ScanFor({ 0xE8 }, true, 1).RelativeOffset(1).GetAs<void*>();
	addresses::unsafeenvironnement = Memcury::Scanner::FindStringRef(UNSAFE_ENVIRONNEMENT).ScanFor({ WILDCARD, WILDCARD, WILDCARD, 0x24, WILDCARD, 0x55 }, false).GetAs<void*>();
	addresses::get_engine_version = Memcury::Scanner::FindStringRef(GET_ENGINE_VERSION).ScanFor({ 0xE8, WILDCARD,WILDCARD,WILDCARD,WILDCARD, 0x4C, 0x8B }, false).RelativeOffset(1).GetAs<void*>();
	if (configuration::bypass_pak_checks)
	{
		addresses::validate_container_signature = Memcury::Scanner::FindPattern("E8 ? ? ? ? 44 39 75 90 0F 85 ? ? ? ?").RelativeOffset(1).GetAs<void*>();
	}

	if (configuration::debug_websockets)
	{
		addresses::lws_client_connect_via_info = Memcury::Scanner::FindPattern("48 89 74 24 ? 57 48 83 EC 20 48 8B 71 30 48 8B F9").GetAs<void*>();
		addresses::lws_create_context = Memcury::Scanner::FindPattern("40 57 48 83 EC 40 48 8B F9").GetAs<void*>();
	}



	log_pointer("curl_easy_setopt", addresses::curl_easy_setopt, true);
	log_pointer("curl_setopt", addresses::curl_setopt, true);
	log_pointer("FEngineVersion::Current", addresses::get_engine_version, true);
	log_pointer("FGenericPlatformMisc::RequestExitWithStatus", addresses::request_exit_with_status, true);
	log_pointer("UnsafeEnvironnement", addresses::unsafeenvironnement, true);
	log_pointer("validate_container_signature", addresses::validate_container_signature, true);
	if (configuration::debug_websockets)
	{
		log_pointer("lws_create_context", addresses::lws_create_context, true);
		log_pointer("lws_client_connect_via_info", addresses::lws_client_connect_via_info, true);
	}
}

void assign_natives(void)
{
	assign(native::curl_easy_setopt, addresses::curl_easy_setopt);
	assign(native::curl_setopt, addresses::curl_setopt);
	assign(native::get_engine_version, addresses::get_engine_version);
	assign(native::request_exit_with_status, addresses::request_exit_with_status);
	assign(native::unsafe_environnement, addresses::unsafeenvironnement);
	assign(native::ValidateContainerSignature, addresses::validate_container_signature);
	if (configuration::debug_websockets)
	{
		assign(native::lws_create_context, addresses::lws_create_context);
		assign(native::lws_client_connect_via_info, addresses::lws_client_connect_via_info);
	}
}