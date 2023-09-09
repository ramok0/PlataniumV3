#include <platanium.hpp>

using namespace platanium::epic::api;
using namespace platanium::authentification::account;



AccountDescriptor account::authentfication::token(const std::string token, const std::pair<const char*, const char*> client, TOKEN_METHOD token_type)
{
	AccountDescriptor result;
	std::stringstream authorization_value;
	authorization_value << "Basic " << auth_clients::build(client);

	platanium::HeaderContainer headers;
	headers.push_back({ "Content-Type", "application/x-www-form-urlencoded" });
	headers.push_back({ "Authorization", authorization_value.str() });

	std::stringstream body_stream;

	std::string grant_type = token_to_body[token_type].first;
	std::string code_property_name = token_to_body[token_type].second;
	body_stream << "grant_type=" << grant_type << "&" << code_property_name << "=" << token;

	cpr::Response response = api::request(endpoints::TOKEN, headers, POST, body_stream.str());
	
	if (response.status_code != 200) {
		spdlog::error("API Response : {} {}", response.status_code, response.text);
		error::set_last_error(error::INVALID_API_RESPONSE);
		return result;
	}
	spdlog::debug("API response : {}", response.text);

	nlohmann::json doc = nlohmann::json::parse(response.text);


	if (!Details::from(doc, result.details)) {
		spdlog::warn("failed to parse details");
		return result;
	}
	
	result.client_id = client.first;
	result.method = token_type;

	return result;
}

const std::optional<platanium::epic::api::account::authentfication::ClientCredentials> platanium::epic::api::account::authentfication::create_device_code(void) {
	const std::optional<std::string> client_creds = client_credentials(platanium::epic::api::auth_clients::fortniteNewSwitchGameClient);

	if (!client_creds) return std::nullopt;

	platanium::HeaderContainer headers;
	headers.push_back({ "Content-Type", "application/x-www-form-urlencoded" });
	headers.push_back({ "Authorization", "bearer " +  *client_creds});

	cpr::Response response = api::request(endpoints::CREATE_DEVICE_CODE, headers, METHOD::POST, "prompt=login");

	if (response.status_code != 200)
	{
		spdlog::debug("API response : {}", response.text);
		error::set_last_error(error::INVALID_API_RESPONSE);
		return std::nullopt;
	}

	nlohmann::json doc = nlohmann::json::parse(response.text);

	platanium::epic::api::account::authentfication::ClientCredentials result;

	int expires_in;

	if (!json::extract_json(doc, "user_code", &result.user_code) ||
		!json::extract_json(doc, "device_code", &result.device_code) ||
		!json::extract_json(doc, "verification_uri_complete", &result.verification_uri_complete) ||
		!json::extract_json(doc, "expires_in", &expires_in)) {
		return std::nullopt;
	}
	
	result.expires_at = std::chrono::system_clock::now() + std::chrono::seconds(expires_in);

	return result;
}


const std::optional<std::string> platanium::epic::api::account::authentfication::client_credentials(const AuthClient auth_client)
{
	std::stringstream authorization_value;
	authorization_value << "Basic " << auth_clients::build(auth_client);

	platanium::HeaderContainer headers;
	headers.push_back({ "Content-Type", "application/x-www-form-urlencoded" });
	headers.push_back({ "Authorization", authorization_value.str() });

	cpr::Response response = api::request(endpoints::TOKEN, headers, POST, "grant_type=client_credentials");
	if (response.status_code != 200) {
		spdlog::error("API Response : {} {}", response.status_code, response.text);
		error::set_last_error(error::INVALID_API_RESPONSE);
		return std::nullopt;
	}

	nlohmann::json doc = nlohmann::json::parse(response.text);


	std::string result;

	if (!json::extract_json<std::string>(doc, "access_token", &result)) return std::nullopt;

	return result;
}

const std::string platanium::epic::api::account::authentfication::exchange(const std::string& access_token)
{
	std::stringstream authorization_value;
	authorization_value << "Bearer " << access_token;

	platanium::HeaderContainer headers;
	headers.push_back({ "Authorization", authorization_value.str() });

	cpr::Response response = api::request(endpoints::EXCHANGE, headers, GET, std::string());

	if (response.status_code != 200)
	{
		spdlog::error("Failed to get exchange code, invalid api response");
		error::set_last_error(error::INVALID_API_RESPONSE);
		return std::string();
	}

	nlohmann::json document = nlohmann::json::parse(response.text);

	std::string result;
	
	if (!json::extract_json(document, "code", &result)) {
		error::set_last_error(error::MISSING_ENTRY);
		return std::string();
	}

	return result;
}

std::optional<platanium::epic::api::account::authentfication::DeviceCode> platanium::epic::api::account::authentfication::get_device_code(void)
{
	platanium::HeaderContainer headers;
	headers.push_back({ "Content-Type", "application/x-www-form-urlencoded" });
	//headers.insert();

	cpr::Response response = api::request(endpoints::CREATE_DEVICE_CODE, headers, METHOD::POST, "prompt=login");
}
