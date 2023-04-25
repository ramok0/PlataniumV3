#include "../include/plataniumv3launcher.hpp"

static bool jsonExists(nlohmann::json& document, const std::string key)
{
	return document.find(key) != document.end();
}

#define json_get(document, key, type, out) \
	if(document.find(key) != document.end())\
	{\
		out = document[key].get<type>();\
	}\
	else {\
		return false;\
	}


bool parse_epic_account(nlohmann::json& document, epic_account_t* out)
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

	spdlog::debug("expires_at => {}, current => {}", out->expires_at, currentTimestamp);

	return true;
}

bool parse_deviceauth(nlohmann::json& document, epic_device_auth_t* out, std::string& secret)
{
	json_get(document, "deviceId", std::string, out->device_id);
	json_get(document, "accountId", std::string, out->account_id);
	if (document["secret"].is_string())
	{
		json_get(document, "secret", std::string, secret);
	}
	else {
		spdlog::error("Type of deviceauth's secret is not std::string, cannot parse it");
		return false;
	}

	return true;
}

bool parse_configuration(nlohmann::json& document, configuration_t* out)
{
	json_get(document, "disableSSL", bool, out->disableSSL)
		json_get(document, "detourURL", bool, out->detourURL)
		json_get(document, "useProxy", bool, out->useProxy)
		json_get(document, "dump_aes", bool, out->dump_aes)
		json_get(document, "should_check_pak", bool, out->should_check_pak)
		json_get(document, "forwardHost", std::string, out->forwardHost)
		json_get(document, "forwardProxy", std::string, out->forwardProxy)
		json_get(document, "forwardPort", int, out->forwardPort)
		json_get(document, "fortnite_path", std::string, out->fortnite_build.path)
		if(!out->fortnite_build.path.empty())
			find_fortnite_engine_version();

		if (document.find("device_auth") != document.end())
		{
			json_get(document["device_auth"], "account_id", std::string, out->deviceAuth.account_id);
			json_get(document["device_auth"], "device_id", std::string, out->deviceAuth.device_id);

			if (document["device_auth"]["secret"].is_array())
			{
				out->deviceAuth.secret = document["device_auth"]["secret"].get<std::vector<uint8_t>>();
			}
		}

	return true;
}