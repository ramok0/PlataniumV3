#include <platanium.hpp>

std::shared_ptr<platanium::authentification::account::Account> platanium::authentification::managers::EpicAuthorizationCodeAuthManager::login(const Credentials& creds)
{
	return this->epic_login(creds);
}