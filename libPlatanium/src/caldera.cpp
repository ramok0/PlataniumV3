#include <platanium.hpp>

const platanium::authentification::account::EpicGamesAccount::Caldera platanium::epic::api::caldera::racp(const std::string& account_id, const std::string& exchange_code, const std::string epic_app)
{
	HeaderContainer headers;
	headers.push_back(std::make_pair("Content-Type", "application/json"));

	nlohmann::json body = {
		{"account_id", account_id},
		{"exchange_code", exchange_code},
		{"test_mode", false},
		{"epic_app", epic_app},
		{"nvidia", false},
		{"luna", false}
	};

	platanium::authentification::account::EpicGamesAccount::Caldera out;

	cpr::Response response = platanium::epic::api::request(epic::api::endpoints::RACP, headers, POST, body.dump());
	
	if (response.status_code != 200) {
		spdlog::error("RASP failed with status code {}", response.status_code);
		error::set_last_error(error::INVALID_API_RESPONSE);
		return out;
	}

	nlohmann::json data = nlohmann::json::parse(response.text);

	return authentification::account::EpicGamesAccount::Caldera::from(data);
}