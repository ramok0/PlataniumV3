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

bool platanium::authentification::account::EpicGamesAccount::create_device_auth(Credentials& out)
{
	if (!platanium::exchange_to(platanium::epic::api::auth_clients::fortniteIOSGameClient)) return false;

	out.client_id = platanium::authentification::account::get_current_account()->get_client_id();
	out.type = EPIC_DEVICE_AUTH;

	std::string url = platanium::epic::api::endpoints::create_device_auth();

	platanium::HeaderContainer headers;
	headers.push_back({ "Content-Type", "application/json" });
	headers.push_back({ "Authorization", "bearer " + platanium::authentification::account::get_current_account()->get_details().access_token });

	cpr::Response response = platanium::epic::api::request(url, headers, platanium::epic::api::METHOD::POST, "{}");

//	spdlog::debug("Device Auth response => {}", response.text);

	if (response.status_code != 200) return false;

	nlohmann::json data = nlohmann::json::parse(response.text);

	if (!Credentials::from(data, out)) return false;

	return true;
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
	platanium::exchange_to(platanium::epic::api::auth_clients::launcherAppClient2);

	std::string exchange_code = this->get_exchange_code();

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