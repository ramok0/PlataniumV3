#include "../include/platanium.hpp"
#include <cxxurl/url.hpp>
#include <minhook/MinHook.h>
#include <array>
#pragma comment(lib, "lib/minhook/minhook.lib")

#define opthook(opt, condition, firstmodif) \
    case opt: \
        if (condition) { \
            firstmodif(); \
        } else { \
            goto defaultBehavior; \
        } \




CURLcode __fastcall hk_curl_easy_setopt(void* curlhandle, CURLoption opt, ...)
{
	va_list arg;
	va_start(arg, opt);

	if (!curlhandle) {
		return CURLE_BAD_FUNCTION_ARGUMENT;
	}

	CURLcode result{};

	auto setProxy = [&result, &curlhandle]() {
		if (configuration::useProxy)
		{
			std::cout << std::format("[plataniumv3] - settings proxy to {}", configuration::forwardProxy) << std::endl;
			curl_setopt(curlhandle, CURLOPT_PROXY, configuration::forwardProxy.c_str());
		}
	};

	auto disableSSLPeer = [&result, &curlhandle]() {
		result = curl_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, 0);
	};

	auto disableSSLHost = [&result, &curlhandle]() {
		result = curl_setopt(curlhandle, CURLOPT_SSL_VERIFYHOST, 0);
	};

	auto setUrl = [&result, &curlhandle, &arg]() {
		std::string surl = va_arg(arg, char*);

		Url url{ surl };

		std::string host = url.host();

		if (configuration::detourURL && (host.ends_with("ol.epicgames.com") || host.ends_with(".akamaized.net") || host.ends_with("on.epicgames.com")))
		{
			url.scheme("http").host(configuration::forwardHost).port(configuration::forwardPort);
			std::cout << std::format("[plataniumv3] - forwarding request to {}", configuration::forwardHost) << std::endl;
		}

		std::string url_result = url.str();

		result = curl_setopt(curlhandle, CURLOPT_URL, url_result.c_str());
	};

	switch (opt) {
		opthook(CURLOPT_SSL_VERIFYPEER, configuration::disableSSL, disableSSLPeer);
		setProxy();
		break;
		opthook(CURLOPT_SSL_VERIFYHOST, configuration::disableSSL, disableSSLHost);
		break;
		opthook(CURLOPT_PROXY, configuration::useProxy, setProxy);
		break;
		opthook(CURLOPT_URL, configuration::detourURL, setUrl);
		break;
	default:
	defaultBehavior:
		result = native::curl_setopt(curlhandle, opt, arg);
		va_end(arg);
		break;
	}

	va_end(arg);

	return result;
}

void hk_request_exit_with_status(bool, uint8_t)
{
	std::cout << "[plataniumv3] - successfully bypassed request exit with status" << std::endl;
	return;
}

__int64 hk_unsafe_environnement(__int64* a1, char a2, __int64 a3, char a4)
{
	std::cout << "[plataniumv3] - successfully bypassed unsafeenvironnement popup" << std::endl;
	return 0;
}


inline FIoStatus hkValidateContainerSignature(__int64 a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6)
{
	std::cout << "[plataniumv3] - replaced ValidateContainerSignature response succesfully" << std::endl;
	return { EIoErrorCode::Ok, (TCHAR*)TEXT("OK")};
}

void place_hooks(void)
{
	MH_Initialize();

	if (configuration::detourURL || configuration::disableSSL || configuration::useProxy)
	{
		MH_CreateHook((void*&)native::curl_easy_setopt, hk_curl_easy_setopt, nullptr);
	}

	MH_CreateHook((void*&)native::request_exit_with_status, hk_request_exit_with_status, nullptr);
	MH_CreateHook((void*&)native::unsafe_environnement, hk_unsafe_environnement, nullptr);
	if (configuration::bypass_pak_checks)
	{
		//ValidateContainerSignature
		MH_CreateHook((void*&)native::ValidateContainerSignature, hkValidateContainerSignature, nullptr);
	}
	MH_EnableHook(MH_ALL_HOOKS);
}