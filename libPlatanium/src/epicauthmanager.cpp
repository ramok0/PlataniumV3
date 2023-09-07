#include <platanium.hpp>

std::shared_ptr<platanium::authentification::account::Account> platanium::authentification::managers::EpicAuthManager::epic_login(std::string token, std::string client_id)
{
	auto it = std::find_if(platanium::auth_clients.begin(), platanium::auth_clients.end(), [&client_id](std::pair<const char*, const char*>& auth_client) {
		return auth_client.first == client_id;
		});

	if (it == platanium::auth_clients.end())
	{
		spdlog::error("Failed to login because the issued client id is not supported by PlataniumV3");
		return nullptr;
	}

	platanium::authentification::account::AccountDescriptor descriptor = platanium::epic::api::account::authentfication::token(token, *it, this->m_type);

	if (descriptor)
	{
		spdlog::debug("EpicLogin with AuthManager : {}", this->get_name());
		descriptor.authManagerName = this->get_name();
		descriptor.unique_authorization = this->bSupportsMultipleConnections;
		return std::make_shared<account::EpicGamesAccount>(descriptor);
	}
	else {
		return nullptr;
	}
}