#include "../include/plataniumv3launcher.hpp"
#include <base64.hpp>
#include <cpr/cpr.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")

bool epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t* out)
{
	if (authorizationCode.size() != 32) return false;

	cpr::Response response = cpr::Post(cpr::Url(EPIC_GENERATE_TOKEN_URL), cpr::Payload{ {"grant_type", "authorization_code"}, {"code", authorizationCode} }, cpr::Header{ {"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)} });
	if (response.status_code != 200) return false;

	nlohmann::json body = nlohmann::json::parse(response.text);

	if (!parse_epic_account(body, out)) return false;

	*current_epic_account = out;

	spdlog::info("Successfully connected to epicgames with authorization_code");

	return true;
}

bool epic_login_with_device_auth(epic_device_auth_t device_auth, epic_account_t* out)
{
	if (device_auth.account_id.empty() || device_auth.device_id.empty() || device_auth.secret.empty()) return false;

	cpr::Response response = cpr::Post(
		cpr::Url(EPIC_GENERATE_TOKEN_URL), 
		cpr::Header{ {"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)} }, 
		cpr::Payload{ {"grant_type", "device_auth"}, {"account_id", device_auth.account_id}, {"device_id", device_auth.device_id}, {"secret", device_auth.secret } 
	});

	if (response.status_code != 200) return false;
	
	nlohmann::json body = nlohmann::json::parse(response.text);

	if (!parse_epic_account(body, out)) return false;

	*current_epic_account = out;

	spdlog::info("Successfully connected to epicgames with deviceauth");

	return true;
}

bool epic_create_device_auth(epic_device_auth_t* out)
{
	if (!*current_epic_account) return false;

	cpr::Response response = cpr::Post(
		cpr::Url(std::format(EPIC_GENERATE_DEVICE_AUTH, (*current_epic_account)->account_id)),
		cpr::Header{ {"Authorization", epic_generate_bearer_authorization()}, {"Content-Type", "application/json"}}
	);

	spdlog::debug("Response => {}", response.text);

	if (response.status_code != 200) return false;

	nlohmann::json json = nlohmann::json::parse(response.text);

	if (!parse_deviceauth(json, out)) return false;

	return true;
}

std::string epic_generate_bearer_authorization(void)
{
	return std::format("Bearer {}", (*current_epic_account)->access_token);
}

std::string epic_create_basic_authorization(std::string client_id, std::string client_secret)
{
	return std::format("Basic {}", macaron::Base64::Encode(client_id + ":" + client_secret));
}

