#include <platanium.hpp>

nlohmann::json platanium::authentification::account::AccountDescriptor::to_json(void)
{
	nlohmann::json data = {
		{"details", this->details.to_json()},
		{"method", this->method},
		{"auth_manager", this->authManagerName},
		{"client_id", this->client_id}
	};

	return data;
}
