#include "../include/plataniumv3launcher.hpp"

#define json_get(document, key, type, out) \
	if(document.find(key) != document.end())\
	{\
		out = document[key].get<type>();\
	}\
	else {\
		spdlog::info("missing key {}", key);\
		return PLATANIUM_FAILURE_REASON::PLATANIUM_JSON_MISSING_KEY;\
	}


PLATANIUM_FAILURE_REASON parse_epic_account(nlohmann::json& document, epic_account_t* out)
{
	int expires_in = 0;
	int refresh_expires_in = 0;

	json_get(document, "displayName", std::string, out->display_name);
	json_get(document, "access_token", std::string, out->access_token);
	json_get(document, "refresh_token", std::string, out->refresh_token);
	json_get(document, "account_id", std::string, out->account_id);
	json_get(document, "refresh_expires", int, refresh_expires_in);
	json_get(document, "expires_in", int, expires_in);

	int currentTimestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	out->expires_at = currentTimestamp + expires_in;
	out->refresh_expires_at = currentTimestamp + refresh_expires_in;

	spdlog::debug("{} - expires_at => {}, current => {}", __FUNCTION__, out->expires_at, currentTimestamp);

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON parse_deviceauth(nlohmann::json& document, epic_device_auth_t* out, std::string& secret)
{
	json_get(document, "deviceId", std::string, out->device_id);
	json_get(document, "accountId", std::string, out->account_id);
	if (document["secret"].is_string())
	{
		json_get(document, "secret", std::string, secret);
	}
	else {
		spdlog::error("{} - Type of deviceauth's secret is not std::string, cannot parse it", __FUNCTION__);
		return PLATANIUM_FAILURE_REASON::PLATANIUM_JSON_INVALID;
	}

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON parse_configuration(nlohmann::json& document, configuration_t* out)
{
	json_get(document, "disableSSL", bool, out->disableSSL)
		json_get(document, "detourURL", bool, out->detourURL)
		json_get(document, "useProxy", bool, out->useProxy)
		json_get(document, "dump_aes", bool, out->dump_aes)
		json_get(document, "debug_websockets", bool, out->debug_websockets)
		json_get(document, "should_check_pak", bool, out->should_check_pak)
		json_get(document, "forwardHost", std::string, out->forwardHost)
		json_get(document, "forwardProxy", std::string, out->forwardProxy)
		json_get(document, "forwardPort", int, out->forwardPort)
		json_get(document, "fortnite_path", std::string, out->fortnite_build.path);


	//update old config.json file
	if (document.find("no_dll") == document.end()) {
		document["no_dll"] = false;
	}

	if (document.find("use_custom_arguments") == document.end())
	{
		document["use_custom_arguments"] = false;
		document["custom_arguments"] = "";
	}

	json_get(document, "no_dll", bool, g_configuration->no_dll);
	json_get(document, "use_custom_arguments", bool, out->use_custom_arguments);
	json_get(document, "custom_arguments", std::string, out->custom_arguments);

	if (!out->fortnite_build.path.empty())
	{
		PLATANIUM_FAILURE_REASON fortnite_find_verison_failure_reason = platalog_error(find_fortnite_engine_version(), "find_fortnite_engine_version");
		if (!PLATANIUM_OK(fortnite_find_verison_failure_reason))
		{
			return PLATANIUM_FAILURE_REASON::PLATANIUM_FAILED_TO_PARSE;
		}
	}

	if (document.find("device_auth") != document.end())
	{
		json_get(document["device_auth"], "account_id", std::string, out->deviceAuth.account_id);
		json_get(document["device_auth"], "device_id", std::string, out->deviceAuth.device_id);

		if (document["device_auth"]["secret"].is_string())
		{
			out->deviceAuth.secret = document["device_auth"]["secret"].get<std::string>();
		}
	}

	return PLATANIUM_NO_FAILURE;
}