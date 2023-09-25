#include <platanium.hpp>
#include <fstream>
#include <array>
#include <iostream>

platanium::Configuration::Data platanium::Configuration::get_default()
{
	Data default_data;

	memset(&default_data, 0, sizeof(default_data));

	default_data.fortnite_path = platanium::epic::Fortnite::find_path().string();

	return default_data;
}

bool platanium::Configuration::load()
{
	if (this->data) {
		set_last_error(platanium::error::ALREADY_EXISTS);
		return false;
	}

	this->data = new Data();

	memset(this->data, 0, sizeof(Data));

	std::string buffer;
	if (!this->read(buffer)) {
		spdlog::error("failed to read configuration, last error : {}", (int)error::get_last_error());
		return false;
	}

	if (!this->parse(buffer)) {
		spdlog::error("failed to parse configuration, last error : {}", (int)error::get_last_error());
		return false;
	}

	spdlog::info("configuration has been loaded successfully");

	return true;
}

bool platanium::Configuration::load_default()
{
	this->data = new Data();
	*this->data = this->get_default();

	this->write();

	return true;
}

bool platanium::Configuration::write()
{
	Data* data = this->get();

	nlohmann::json doc;
	doc[xorstr_("url")] = data->url;
	doc[xorstr_("proxy")] = data->proxy;
	doc[xorstr_("bRedirectTraffic")] = data->bRedirectTraffic;
	doc[xorstr_("bUseProxy")] = data->bUseProxy;
	doc[xorstr_("port")] = data->port;
	doc[xorstr_("fortnite_path")] = data->fortnite_path;
	
	nlohmann::json accounts = nlohmann::json::array();

	for (int i = 0; i < platanium::accounts.size(); i++)
	{
		std::pair<std::string, platanium::authentification::Credentials> account = platanium::accounts[i];

		std::string out;
		if (!account.second.cipher(out)) {
			spdlog::error(xorstr_("Failed to cipher account credentials"));
			continue;
		}


		accounts[i][xorstr_("display_name")] = account.first;
		accounts[i][xorstr_("secret")] = out;
	}

	doc[xorstr_("accounts_ciphered")] = accounts;

	spdlog::debug("writing configuration to {}", this->m_config_path.string());

	std::ofstream stream(this->m_config_path);

	if (!stream.is_open())
	{
		set_last_error(platanium::error::FAILED_TO_WRITE_FILE);
		spdlog::error("failed to write configuration, os last error : ", GetLastError());
		return false;
	}

	stream << doc.dump(4) << std::flush;

	stream.close();

	return true;
}

bool platanium::Configuration::read(std::string& out)
{
	std::ifstream stream(this->m_config_path);

	if (!stream.is_open())
	{
		set_last_error(platanium::error::FAILED_TO_READ_FILE);
		spdlog::error(xorstr_("failed to read configuration"));
		return false;
	}

	std::string buffer;
	while (std::getline(stream, buffer))
	{
		out += buffer;
	}

	stream.close();

	return true;
}

bool platanium::Configuration::parse(const std::string& str)
{
	nlohmann::json doc;
	try {
		doc = nlohmann::json::parse(str);
		spdlog::info(xorstr_("parsed configuration successfully"));
	}
	catch (const nlohmann::json::parse_error&) {
		set_last_error(platanium::error::JSON_PARSING_ERROR);
		spdlog::error(xorstr_("failed to json parse configuration"));
		return false;
	}

	return extract_fields(doc);
}

bool platanium::Configuration::extract_fields(const nlohmann::json& doc)
{
	if (!json::extract_json(doc, xorstr_("url"), &this->data->url)
		|| !json::extract_json(doc, xorstr_("proxy"), &this->data->proxy)
		|| !json::extract_json(doc, xorstr_("bRedirectTraffic"), &this->data->bRedirectTraffic)
		|| !json::extract_json(doc, xorstr_("bUseProxy"), &this->data->bUseProxy)
		|| !json::extract_json(doc, xorstr_("fortnite_path"), &this->data->fortnite_path)
		|| !json::extract_json(doc, xorstr_("port"), &this->data->port))
	{
		set_last_error(platanium::error::MISSING_ENTRY);
		return false;
	}

	if (json::json_exists(doc, xorstr_("accounts_ciphered")))
	{
		for (nlohmann::json document : doc[xorstr_("accounts_ciphered")])
		{
			std::string display_name = document[xorstr_("display_name")].get<std::string>();
			std::string secret = document[xorstr_("secret")].get<std::string>();

			platanium::authentification::Credentials credentials;
			if(platanium::authentification::Credentials::uncipher(secret, credentials))
				platanium::accounts.push_back(std::make_pair(display_name, credentials));
		}
	}

	return true;
}

bool platanium::Configuration::exists()
{
	return std::filesystem::exists(this->m_config_path);
}
