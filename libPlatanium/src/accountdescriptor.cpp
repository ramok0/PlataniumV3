#include <platanium.hpp>

nlohmann::json platanium::authentification::account::AccountDescriptor::to_json(void)
{
	nlohmann::json data = {
		{xorstr_("details"), this->details.to_json()},
		{xorstr_("method"), this->method},
		{xorstr_("auth_manager"), this->authManagerName},
		{xorstr_("client_id"), this->client_id}
	};

	return data;
}
