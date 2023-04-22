#include "../include/plataniumv3launcher.hpp"
#include <base64.hpp>

bool epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t& out)
{
	throw std::runtime_error("Non-implemented");
	return false;
}

bool epic_login_with_device_auth(epic_device_auth_t& device_auth, epic_account_t& out)
{
	throw std::runtime_error("Non-implemented");
	return false;
}

std::string epic_create_basic_authorization(std::string& client_id, std::string& client_secret)
{
	return std::format("Basic {}", macaron::Base64::Encode(client_id + ":" + client_secret));
}