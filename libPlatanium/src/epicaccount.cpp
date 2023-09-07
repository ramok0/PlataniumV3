#include <platanium.hpp>

using namespace platanium::authentification::account;

const EpicGamesAccount::Caldera platanium::authentification::account::EpicGamesAccount::request_anti_cheat_provider(void)
{
	const std::string exchange_code = this->get_exchange_code();
	const EpicGamesAccount::Caldera caldera = platanium::epic::api::caldera::racp(this->get_details().account_id, exchange_code, "fortnite");

	return caldera;
}

const std::string platanium::authentification::account::EpicGamesAccount::get_exchange_code(void)
{
	return epic::api::account::authentfication::exchange(this->get_details().access_token);
}

const platanium::authentification::Credentials platanium::authentification::account::EpicGamesAccount::get_refresh_token()
{
	Credentials data;

	data.refresh_token = this->get_details().refresh_token;
	data.client_id = this->get_client_id();
	data.type = platanium::TOKEN_METHOD::EPIC_REFRESH_TOKEN;

	return data;
}

platanium::ArgumentContainer platanium::authentification::account::EpicGamesAccount::get_authentification_arguments()
{
	spdlog::info("Getting authentification arguments");

	//switch from current client id (this->m_client_id) to launcher token
	std::string exchange_code;
	if (this->get_client_id() == epic::api::auth_clients::launcherAppClient2.first)
	{
		exchange_code = this->get_exchange_code();
	}
	else {
		const std::string current_exchange_code = this->get_exchange_code();
		const AccountDescriptor details = epic::api::account::authentfication::token(current_exchange_code, epic::api::auth_clients::launcherAppClient2, EPIC_EXCHANGE_CODE);

		EpicGamesAccount* account = new EpicGamesAccount(details);

		exchange_code = account->get_exchange_code();

		delete account;
	}

	const EpicGamesAccount::Caldera caldera = this->request_anti_cheat_provider();

	platanium::ArgumentContainer arguments;
	arguments.push_back(std::make_pair("AUTH_PASSWORD", exchange_code));
	arguments.push_back(std::make_pair("epicusername", this->get_details().display_name));
	arguments.push_back(std::make_pair("epicuserid", this->get_details().account_id));
	arguments.push_back(std::make_pair("fromfl", caldera.provider));
	arguments.push_back(std::make_pair("caldera", caldera.jwt));
	arguments.push_back(std::make_pair("plataniumconfigpath", platanium::configuration->get_path().string()));

	return arguments;
}

bool EpicGamesAccount::refresh()
{
	bool success = platanium::login(this->get_refresh_token());
	spdlog::info("Refreshed account result : {}", success);
	return success;
}

const EpicGamesAccount::Caldera EpicGamesAccount::Caldera::from(const nlohmann::json& doc)
{
	static std::map<std::string, std::string> provider_translator = {
		{"EasyAntiCheat", "eac"},
		{"EasyAntiCheatEOS", "eaceos"},
		{"BattlEye", "be"}
	};
	std::string raw_provider;

	EpicGamesAccount::Caldera out;

	bool extract_success = json::extract_json(doc, "jwt", &out.jwt) && json::extract_json(doc, "provider", &raw_provider);
	if (!extract_success) return out; //null opt

	out.provider = provider_translator[raw_provider];
	
	return out;
}