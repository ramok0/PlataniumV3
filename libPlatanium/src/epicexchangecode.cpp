#include <platanium.hpp>

std::shared_ptr<platanium::authentification::account::Account> platanium::authentification::managers::EpicExchangeCodeAuthManager::login(const Credentials& creds)
{
	return this->epic_login(creds.exchange_code, creds.client_id);
}