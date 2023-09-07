#include <platanium.hpp>

std::shared_ptr<platanium::authentification::managers::BaseAuthManager> platanium::get_auth_manager(const std::string authManagerName)
{
	auto it = std::find_if(platanium::auth_managers.begin(), platanium::auth_managers.end(), [&authManagerName](std::shared_ptr<platanium::authentification::managers::BaseAuthManager> manager) {
		return manager->get_name() == authManagerName;
		});

	if (it == platanium::auth_managers.end()) return nullptr;

	return *it;
}

std::shared_ptr<platanium::authentification::managers::BaseAuthManager> platanium::get_auth_manager(platanium::TOKEN_METHOD token_type)
{
	auto it = std::find_if(platanium::auth_managers.begin(), platanium::auth_managers.end(), [&token_type](std::shared_ptr<platanium::authentification::managers::BaseAuthManager> manager) {
		return manager->get_method() == token_type;
		});

	if (it == platanium::auth_managers.end()) return nullptr;

	return *it;
}

