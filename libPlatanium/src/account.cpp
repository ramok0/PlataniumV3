#include <platanium.hpp>
using namespace platanium::authentification::account;

bool Details::from(const nlohmann::json& doc, Details& out)
{
	if (json::json_exists(doc, "displayName")) {
		json::extract_json(doc, "displayName", &out.display_name);
	}
	
	
	if (json::json_exists(doc, "display_name"))
	{
		json::extract_json(doc, "display_name", &out.display_name);
	}

	if (!json::extract_json(doc, "access_token", &out.access_token) ||
	!json::extract_json(doc, "refresh_token", &out.refresh_token) ||
		!json::extract_json(doc, "account_id", &out.account_id)) {
		error::set_last_error(error::MISSING_ENTRY);
		return false;
	}

	return true;
}

nlohmann::json platanium::authentification::account::Details::to_json(void)
{
	nlohmann::json data = {
		{"account_id", this->account_id},
		{"display_name", this->display_name},
		{"refresh_token", this->refresh_token},
	};

	return data;
}

bool platanium::authentification::account::is_account_valid(void)
{
	if (!platanium::currentAccount) return false;
	return platanium::currentAccount->is_valid();
}

std::shared_ptr<Account> platanium::authentification::account::get_current_account(void) {
	if (!platanium::authentification::account::is_account_valid()) throw std::runtime_error("Invalid account");
	return platanium::currentAccount;
}

void platanium::authentification::account::set_current_account(std::shared_ptr<Account> account) {
	platanium::currentAccount = account;
}

bool Account::save() {
	//if (this->get_descriptor().unique_authorization == false)
	//{
	//	if (!this->refresh()) return false;
	//}

	platanium::authentification::Credentials credentials;
	credentials.type = EPIC_DEVICE_AUTH;
	credentials.client_id = this->get_client_id();


	if (!this->create_device_auth(credentials)) return false;

	std::string display_name = platanium::currentAccount->m_descriptor.details.display_name;
	auto it = std::find_if(platanium::accounts.begin(), platanium::accounts.end(), [&display_name](std::pair<std::string, platanium::authentification::Credentials> credentials) {
		return credentials.first == display_name;
		});

	if (it == platanium::accounts.end())
	{
		platanium::accounts.push_back(std::make_pair(platanium::currentAccount->m_descriptor.details.display_name, credentials));
	}
	else {
		it->second = platanium::currentAccount->get_refresh_token();
	}

	return true;
}