#include "../include/plataniumv3launcher.hpp"

static bool jsonExists(nlohmann::json& document, const std::string key)
{
	return document.find(key) != document.end();
}

#define json_get(document, key, type, out) \
	if(document.find(key) != document.end())\
		out = document[key].get<type>();


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

bool parse_configuration(nlohmann::json& document, configuration_t* out)
{
	throw std::runtime_error("Non-implemented");
	return false;
}