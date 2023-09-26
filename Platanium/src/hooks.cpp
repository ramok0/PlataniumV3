#include "../include/platanium.h"
#include <iostream>
#include <memcury/memcury.h>

void Platanium::place_hooks( void )
{

}

CURLcode __fastcall Platanium::hkCurlEasySetopt(void* curl, CURLoption opt, ...)
{
	va_list arg;
	va_start(arg, opt);

	if (!curl) {
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}

	CURLcode result = CURLE_GOT_NOTHING;


	if (opt == CURLOPT_URL)
	{
		std::string sUrl = va_arg(arg, char*);

		spdlog::debug("URL : {}", sUrl);

		result = g_application->curl_setopt_w(curl, CURLOPT_URL, sUrl.c_str());
		va_end(arg);
		return result;
	}

	result = g_application->curl_setopt(curl, opt, arg);

	va_end(arg);

	return result;
}
