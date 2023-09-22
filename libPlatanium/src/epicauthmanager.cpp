#include <platanium.hpp>

std::shared_ptr<platanium::authentification::account::Account> platanium::authentification::managers::EpicAuthManager::epic_login(const Credentials& creds)
{
	std::string client_id = creds.client_id;

	auto it = std::find_if(platanium::auth_clients.begin(), platanium::auth_clients.end(), [&client_id](std::pair<const char*, const char*>& auth_client) {
		return auth_client.first == client_id;
		});

	if (it == platanium::auth_clients.end())
	{
		spdlog::error("Failed to login because the issued client id ({}) ({}) is not supported by PlataniumV3", client_id, client_id.size());
		return nullptr;
	}

	const std::string body = this->get_token_body(creds);

//	spdlog::debug("Body : {}", body);
	spdlog::debug("Client : {}", client_id);

	std::optional<platanium::authentification::account::AccountDescriptor> descriptor = platanium::epic::api::account::authentfication::token(creds, body);

	if (descriptor)
	{
		platanium::authentification::account::AccountDescriptor result = *descriptor;

		spdlog::debug("EpicLogin with AuthManager : {}", this->get_name());
		result.authManagerName = this->get_name();
		result.unique_authorization = this->bSupportsMultipleConnections;
		result.is_epic = true;
		return std::make_shared<account::EpicGamesAccount>(result);
	}
	else {
		return nullptr;
	}
}