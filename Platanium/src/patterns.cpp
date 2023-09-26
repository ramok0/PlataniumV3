#include "../include/platanium.h"
#include <memcury/memcury.h>

void Platanium::find_patterns(void)
{
	this->get_engine_version = Memcury::Scanner::FindStringRef(patterns::GET_ENGINE_VERSION).ScanFor("E8 ? ? ? ? 4C 8B", false).RelativeOffset(1).GetAs<decltype(this->get_engine_version)>();

	Memcury::Scanner curl_easy_setopt = Memcury::Scanner::FindPattern(patterns::CURL_EASY_SETOPT);

	this->curl_easy_setopt = curl_easy_setopt.GetAs<decltype(this->curl_easy_setopt)>();
	this->curl_setopt = curl_easy_setopt.ScanFor({ Memcury::ASM::CALL }).RelativeOffset(1).GetAs<decltype(this->curl_setopt)>();

	spdlog::debug("GetEngineVersion : {}", (void*)this->get_engine_version);
	spdlog::debug("CurlEasySetopt : {}", (void*)this->curl_easy_setopt);
	spdlog::debug("CurlSetopt : {}", (void*)this->curl_setopt);
}
