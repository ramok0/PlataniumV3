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

	return true;
}

bool epic_login_with_device_auth(epic_device_auth_t& device_auth, epic_account_t& out)
{
	throw std::runtime_error("Non-implemented");
	return false;
}

std::string epic_create_basic_authorization(std::string client_id, std::string client_secret)
{
	return std::format("Basic {}", macaron::Base64::Encode(client_id + ":" + client_secret));
}