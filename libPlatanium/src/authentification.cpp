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
