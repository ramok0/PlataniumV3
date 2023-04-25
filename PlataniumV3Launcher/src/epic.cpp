#include "../include/plataniumv3launcher.hpp"
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

	spdlog::info("{} - Successfully connected to epicgames with authorization_code", __FUNCTION__);

	return true;
}

bool epic_login_with_refresh_token(void)
{
	if (!current_epic_account || !*current_epic_account) return false;

	int timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if ((*current_epic_account)->refresh_expires_at < timestamp) return false;

	cpr::Response response = cpr::Post(
		cpr::Url(EPIC_GENERATE_TOKEN_URL),
		cpr::Payload{ 
			{"grant_type", "refresh_token"},
			{"refresh_token", (*current_epic_account)->refresh_token}
		},
		cpr::Header{
			{"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)}
		}
	);

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return false;
	}

	nlohmann::json body = nlohmann::json::parse(response.text);

	return parse_epic_account(body, *current_epic_account);
}

bool epic_login_with_device_auth(epic_device_auth_t device_auth, epic_account_t* out)
{
	if (device_auth.account_id.empty() || device_auth.device_id.empty() || device_auth.secret.empty()) return false;

	std::string secretPlainText;

	if (!uncipher_secret(secretPlainText))
	{
		spdlog::error("Failed to decrypt device_auth=>'secret'");
		return false;
	}

	cpr::Response response = cpr::Post(
		cpr::Url(EPIC_GENERATE_TOKEN_URL), 
		cpr::Header{ {"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)} }, 
		cpr::Payload{ {"grant_type", "device_auth"}, {"account_id", device_auth.account_id}, {"device_id", device_auth.device_id}, {"secret", secretPlainText }
	});

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return false;
	}
	
	nlohmann::json body = nlohmann::json::parse(response.text);

	if (!parse_epic_account(body, out)) return false;

	*current_epic_account = out;

	spdlog::info("{} - Successfully connected to epicgames with deviceauth", __FUNCTION__);

	return true;
}

bool epic_create_device_auth(epic_device_auth_t* out)
{
	if (!*current_epic_account) return false;

	cpr::Response response = cpr::Post(
		cpr::Url(std::format(EPIC_GENERATE_DEVICE_AUTH, (*current_epic_account)->account_id)),
		cpr::Header{ {"Authorization", epic_generate_bearer_authorization()}, {"Content-Type", "application/json"}}
	);


	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return false;
	}

	nlohmann::json json = nlohmann::json::parse(response.text);

	std::string secretPlainText;
	if (!parse_deviceauth(json, out, secretPlainText) || !cipher_secret(out, secretPlainText)) return false;

	secretPlainText.clear();

	return true;
}

std::string epic_generate_bearer_authorization(void)
{
	return std::format("Bearer {}", (*current_epic_account)->access_token);
}

std::string epic_create_basic_authorization(std::string client_id, std::string client_secret)
{
	return std::format("Basic {}", base64_encode(client_id + ":" + client_secret));
}

bool epic_create_exchange_code(std::string& out)
{
	cpr::Response response = cpr::Get(
		cpr::Url(EPIC_GENERATE_EXCHANGE_CODE),
		cpr::Header{ {"Authorization", epic_generate_bearer_authorization()} }
	);

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return false;
	}

	nlohmann::json body = nlohmann::json::parse(response.text);

	if (body.find("code") == body.end()) return false;

	out = body["code"].get<std::string>();

	return true;
}