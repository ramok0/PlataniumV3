#include "../include/platanium.hpp"
#include <cxxurl/url.hpp>
#include <minhook/MinHook.h>
#include <array>
#include <fstream>
#include <stacktrace>
#pragma comment(lib, "lib/minhook/minhook.lib")

#define opthook(opt, condition, firstmodif) \
    case opt: \
        if (condition) { \
            firstmodif(); \
        } else { \
            goto defaultBehavior; \
        } \

inline std::vector<std::string> found_keys;


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
		if (configuration::useProxy)
		{
			curl_setopt(curlhandle, CURLOPT_PROXY, configuration::forwardProxy.c_str());
		}
		if (configuration::disableSSL)
		{
			result = curl_setopt(curlhandle, CURLOPT_SSL_VERIFYPEER, 0);
		}
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
		opthook(CURLOPT_SSL_VERIFYPEER, configuration::disableSSL || configuration::useProxy, disableSSLPeer);
		setProxy();
		break;
		opthook(CURLOPT_SSL_VERIFYHOST, configuration::disableSSL, disableSSLHost);
		break;
		opthook(CURLOPT_PROXY, configuration::useProxy, setProxy);
		break;
		opthook(CURLOPT_URL, configuration::detourURL || configuration::disableSSL, setUrl);
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
}

__int64 hk_unsafe_environnement(__int64* a1, char a2, __int64 a3, char a4)
{
	std::cout << "[plataniumv3] - successfully bypassed unsafeenvironnement popup" << std::endl;
	return 0;
}


FIoStatus hkValidateContainerSignature(__int64 a1, __int64 a2, __int64 a3, __int64 a4, __int64 a5, __int64 a6)
{
	std::cout << "[plataniumv3] - replaced ValidateContainerSignature response succesfully" << std::endl;
	return { EIoErrorCode::Ok, (TCHAR*)TEXT("OK")};
}

__int64 __fastcall hk_lws_create_context(__int16* a1)
{
	lws_context_creation_info* context_creation = reinterpret_cast<lws_context_creation_info*>(a1);

	if (context_creation && configuration::useProxy)
	{
		size_t offset = configuration::forwardProxy.find(":");
		std::string host = configuration::forwardProxy.substr(0, offset);
		std::string port = configuration::forwardProxy.substr(configuration::forwardProxy.find(":")+1);
		context_creation->http_proxy_address = host.c_str();
		context_creation->http_proxy_port = (unsigned int)std::stoi(port);

		std::cout << "attached proxy " << host << ":" << port << " into ws successfully" << std::endl;
	}

	return native::o_lws_create_context((short*)context_creation);
}

__int64 __fastcall hk_lws_client_connect_via_info(__int64 a1)
{
	lws_client_connect_info* connect_info = reinterpret_cast<lws_client_connect_info*>(a1);


	if (connect_info)
	{
	//	connect_info->ssl_connection = 2;

		std::cout << "ws path : " << connect_info->path << std::endl;
		std::cout << "ws protocol : " << connect_info->protocol << std::endl;
		std::cout << "ws host : " << connect_info->host << std::endl;
		std::cout << "ws port : " << connect_info->port << std::endl;
		std::cout << "ws address : " << connect_info->address << std::endl;

		//std::cout << std::stacktrace::current() << std::endl;
	}


	return native::o_lws_client_connect_via_info((__int64)connect_info);
}

//	inline __int64(__fastcall* DecryptData)(unsigned __int8*, __int64, const struct FAESKey*);
__int64 hkDecryptData(unsigned __int8* Contents, __int64 NumBytes, FAESKey* KeyBytes) {

	std::string key = KeyBytes->to_hex_string();

	if (std::find(found_keys.begin(), found_keys.end(), key) == found_keys.end()) {
		std::cout << "FOUND AES KEY : " << key << std::endl;

		std::ofstream file;
		file.open("aes_keys.txt", std::ios::app);

		file << key << std::endl;

		file.close();

		found_keys.push_back(key);
	}
	
	return native::o_DecryptData(Contents, NumBytes, KeyBytes);
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

	if (configuration::dump_aes)
	{
		MH_CreateHook((void*&)native::DecryptData, hkDecryptData, (LPVOID*)&native::o_DecryptData);
	}

	if (configuration::debug_websockets)
	{
		MH_CreateHook((void*&)native::lws_create_context, hk_lws_create_context, (LPVOID*)&native::o_lws_create_context);
		//MH_CreateHook((void*&)native::lws_client_connect_via_info, hk_lws_client_connect_via_info, (LPVOID*)&native::o_lws_client_connect_via_info);
	}

	MH_EnableHook(MH_ALL_HOOKS);
}