#include "../include/platanium.hpp"

#define WILDCARD UINT8_MAX
#define assign(dest, address) dest = reinterpret_cast<decltype(dest)>(address);

void find_patterns(void)
{
	Memcury::Scanner curl_easy_setopt = Memcury::Scanner::FindPattern(CURL_EASY_SETOPT);
	addresses::curl_easy_setopt = curl_easy_setopt.GetAs<void*>();
	log_pointer("curl_easy_setopt", addresses::curl_easy_setopt, true);
	addresses::curl_setopt = curl_easy_setopt.ScanFor({ Memcury::ASM::CALL }).RelativeOffset(1).GetAs<void*>();
	log_pointer("curl_setopt", addresses::curl_setopt, true);

	addresses::get_engine_version = Memcury::Scanner::FindStringRef(GET_ENGINE_VERSION).ScanFor({ 0xE8, WILDCARD,WILDCARD,WILDCARD,WILDCARD, 0x4C, 0x8B }, false).RelativeOffset(1).GetAs<void*>();
	log_pointer("FEngineVersion::Current", addresses::get_engine_version, true);

	assign(native::get_engine_version, addresses::get_engine_version);

	float engineVersion = GetEngineVersion();

	const wchar_t* RequestExitWithStatusStr = REQUEST_EXIT_WITH_STATUS;

	if (engineVersion >= 5.3f)
	{
		RequestExitWithStatusStr = L"Cannot find a compatible Vulkan device that supports surface presentation.\n\n";
	}

	std::cout << "Found Engine Version : " << engineVersion << std::endl;

	std::wcout << L"Searching String : " << RequestExitWithStatusStr << std::endl;

	//this is by far the best way to find this function
	addresses::request_exit_with_status = Memcury::Scanner::FindStringRef(RequestExitWithStatusStr).ScanFor({ 0xE8 }, true, 1).RelativeOffset(1).GetAs<void*>();
	log_pointer("FGenericPlatformMisc::RequestExitWithStatus", addresses::request_exit_with_status, true);
	if (engineVersion >= 5.3f)
	{
		addresses::unsafeenvironnement = Memcury::Scanner::FindStringRef(UNSAFE_ENVIRONNEMENT).ScanFor({ 0x40, 0x55, 0x53, 0x56 }, false).GetAs<void*>();
	}
	else {
		addresses::unsafeenvironnement = Memcury::Scanner::FindStringRef(UNSAFE_ENVIRONNEMENT).ScanFor({ WILDCARD, WILDCARD, WILDCARD, 0x24, WILDCARD, 0x55 }, false).GetAs<void*>();
	}
	log_pointer("UnsafeEnvironnement", addresses::unsafeenvironnement, true);
	if (configuration::bypass_pak_checks)
	{
		addresses::validate_container_signature = Memcury::Scanner::FindPattern("E8 ? ? ? ? 44 39 75 90 0F 85 ? ? ? ?").RelativeOffset(1).GetAs<void*>();
		log_pointer("validate_container_signature", addresses::validate_container_signature, true);
	}

	if (configuration::debug_websockets)
	{
		addresses::lws_client_connect_via_info = Memcury::Scanner::FindPattern("48 89 74 24 ? 57 48 83 EC 20 48 8B 71 30 48 8B F9").GetAs<void*>();
		log_pointer("lws_client_connect_via_info", addresses::lws_client_connect_via_info, true);
		addresses::lws_create_context = Memcury::Scanner::FindPattern("40 57 48 83 EC 40 48 8B F9").GetAs<void*>();
		log_pointer("lws_create_context", addresses::lws_create_context, true);
	}

	if (configuration::dump_aes)
	{
		addresses::decrypt_data = Memcury::Scanner::FindPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 49 8B D8 48 8B FA 48 8B F1 E8 ? ? ? ? 4C 8B C7").GetAs<void*>();
		log_pointer("FAES::DecryptData", addresses::decrypt_data, true);
	}
}

void assign_natives(void)
{
	assign(native::curl_easy_setopt, addresses::curl_easy_setopt);
	assign(native::curl_setopt, addresses::curl_setopt);
	assign(native::request_exit_with_status, addresses::request_exit_with_status);
	assign(native::unsafe_environnement, addresses::unsafeenvironnement);
	assign(native::ValidateContainerSignature, addresses::validate_container_signature);

	assign(native::DecryptData, addresses::decrypt_data);
	if (configuration::debug_websockets)
	{
		assign(native::lws_create_context, addresses::lws_create_context);
		assign(native::lws_client_connect_via_info, addresses::lws_client_connect_via_info);
	}
}