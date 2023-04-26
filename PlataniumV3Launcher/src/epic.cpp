#include "../include/plataniumv3launcher.hpp"
#include <cpr/cpr.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")

PLATANIUM_FAILURE_REASON epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t* out)
{
	if (authorizationCode.size() != 32) return PLATANIUM_MISSING_AUTHORIZATION_CODE;

	cpr::Response response = cpr::Post(cpr::Url(EPIC_GENERATE_TOKEN_URL), cpr::Payload{ {"grant_type", "authorization_code"}, {"code", authorizationCode} }, cpr::Header{ {"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)} });
	if (response.status_code != 200) return PLATANIUM_INVALID_RESPONSE_FROM_API;

	nlohmann::json body = nlohmann::json::parse(response.text);

	PLATANIUM_FAILURE_REASON parse_fail_reason = platalog_error(parse_epic_account(body, out), "parse_epic_account");;

	if (!PLATANIUM_OK(parse_fail_reason)) 
		return PLATANIUM_FAILED_TO_PARSE;
	

	*current_epic_account = out;

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON epic_login_with_refresh_token(void)
{
	if (!current_epic_account || !*current_epic_account) return PLATANIUM_MISSING_ACCOUNT;

	int timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	if ((*current_epic_account)->refresh_expires_at < timestamp) return PLATANIUM_EXPIRED_REFRESH_TOKEN;

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
		return PLATANIUM_INVALID_RESPONSE_FROM_API;
	}

	nlohmann::json body = nlohmann::json::parse(response.text);

	PLATANIUM_FAILURE_REASON parse_fail_reason = platalog_error(parse_epic_account(body, *current_epic_account), "parse_epic_account");;

	if (!PLATANIUM_OK(parse_fail_reason)) 
		return PLATANIUM_FAILED_TO_PARSE;
	

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON epic_login_with_device_auth(epic_device_auth_t device_auth, epic_account_t* out)
{
	if (device_auth.account_id.empty() || device_auth.device_id.empty() || device_auth.secret.empty()) return PLATANIUM_MISSING_DEVICE_AUTH;

	std::string secretPlainText;

	if (!PLATANIUM_OK(platalog_error(uncipher_secret(secretPlainText), "uncipher_secret")))
		return PLATANIUM_FAILED_TO_DECRYPT;

	cpr::Response response = cpr::Post(
		cpr::Url(EPIC_GENERATE_TOKEN_URL), 
		cpr::Header{ {"Authorization", epic_create_basic_authorization(FORTNITE_IOS_GAME_CLIENT_ID, FORTNITE_IOS_GAME_CLIENT_SECRET)} }, 
		cpr::Payload{ {"grant_type", "device_auth"}, {"account_id", device_auth.account_id}, {"device_id", device_auth.device_id}, {"secret", secretPlainText }
	});

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return PLATANIUM_INVALID_RESPONSE_FROM_API;
	}
	
	nlohmann::json body = nlohmann::json::parse(response.text);

	PLATANIUM_FAILURE_REASON parse_fail_reason = platalog_error(parse_epic_account(body, out), "parse_epic_account");;

	if (!PLATANIUM_OK(parse_fail_reason))
		return PLATANIUM_FAILED_TO_PARSE;

	*current_epic_account = out;

	spdlog::info("{} - Successfully connected to epicgames with deviceauth", __FUNCTION__);

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON epic_create_device_auth(epic_device_auth_t* out)
{
	if (!*current_epic_account) return PLATANIUM_MISSING_ACCOUNT;

	cpr::Response response = cpr::Post(
		cpr::Url(std::format(EPIC_GENERATE_DEVICE_AUTH, (*current_epic_account)->account_id)),
		cpr::Header{ {"Authorization", epic_generate_bearer_authorization()}, {"Content-Type", "application/json"}}
	);

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return PLATANIUM_INVALID_RESPONSE_FROM_API;
	}

	nlohmann::json json = nlohmann::json::parse(response.text);

	std::string secretPlainText;

	PLATANIUM_FAILURE_REASON parse_failure_reason = platalog_error(parse_deviceauth(json, out, secretPlainText), "parse_deviceauth");

	if (!PLATANIUM_OK(parse_failure_reason))
		return PLATANIUM_FAILED_TO_PARSE;
	

	PLATANIUM_FAILURE_REASON cipher_failure_reason = platalog_error(cipher_secret(out, secretPlainText), "cipher_secret");

	if (!PLATANIUM_OK(cipher_failure_reason))
		return PLATANIUM_FAILED_TO_CRYPT;
	

	secretPlainText.clear();

	return PLATANIUM_NO_FAILURE;
}

std::string epic_generate_bearer_authorization(void)
{
	return std::format("Bearer {}", (*current_epic_account)->access_token);
}

std::string epic_create_basic_authorization(std::string client_id, std::string client_secret)
{
	return std::format("Basic {}", base64_encode(client_id + ":" + client_secret));
}

PLATANIUM_FAILURE_REASON epic_create_exchange_code(std::string& out)
{
	cpr::Response response = cpr::Get(
		cpr::Url(EPIC_GENERATE_EXCHANGE_CODE),
		cpr::Header{ {"Authorization", epic_generate_bearer_authorization()} }
	);

	if (response.status_code != 200) {
		spdlog::debug("{} - Response => {}", __FUNCTION__, response.text);
		return PLATANIUM_INVALID_RESPONSE_FROM_API;
	}

	nlohmann::json body = nlohmann::json::parse(response.text);

	if (body.find("code") == body.end()) return PLATANIUM_JSON_INVALID;

	out = body["code"].get<std::string>();

	return PLATANIUM_NO_FAILURE;
}