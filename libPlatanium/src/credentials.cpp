#include <platanium.hpp>
#include <dpapi.h>



bool platanium::authentification::Credentials::from(const nlohmann::json& in, Credentials& out)
{
	bool fail = false;

	auto extract_key = [&in, &fail]<typename T>(const char* key, T * out) {
		if (!fail)
		{
			if (!json::extract_json(in, key, out))
			{
				spdlog::error(xorstr_("failed to parse credentials from json"));
				fail = true;
				return false;
			}
		}

		return true;
	};

	if (json::json_exists(in, xorstr_("type")))
		json::extract_json(in, xorstr_("type"), &out.type);

	if (json::json_exists(in, xorstr_("client_id")))
		json::extract_json(in, xorstr_("client_id"), &out.client_id);

	switch (out.type) {
	case TOKEN_METHOD::EPIC_AUTHORIZATION_CODE:
		extract_key(xorstr_("authorization_code"), &out.authorization_code);
		break;
	case TOKEN_METHOD::EPIC_DEVICE_AUTH:
		extract_key(xorstr_("secret"), &out.secret);
		extract_key(xorstr_("deviceId"), &out.deviceId);
		extract_key(xorstr_("accountId"), &out.accountId);
		break;
	case TOKEN_METHOD::EPIC_DEVICE_CODE:
		extract_key(xorstr_("device_code"), &out.device_code);
		break;
	case TOKEN_METHOD::EPIC_EXCHANGE_CODE:
		extract_key(xorstr_("exchange_code"), &out.exchange_code);
		break;
	case TOKEN_METHOD::EPIC_REFRESH_TOKEN:
		extract_key(xorstr_("refresh_token"), &out.refresh_token);
		break;
	}

	return !fail;
}

nlohmann::json platanium::authentification::Credentials::to_json(void)
{
	nlohmann::json data = {
		{xorstr_("client_id"), this->client_id},
		{xorstr_("type"), this->type}
	};

	switch (this->type) {
	case TOKEN_METHOD::EPIC_AUTHORIZATION_CODE:
		data[xorstr_("authorization_code")] = this->authorization_code;
		break;
	case TOKEN_METHOD::EPIC_DEVICE_AUTH:
		data[xorstr_("secret")] = this->secret;
		data[xorstr_("deviceId")] = this->deviceId;
		data[xorstr_("accountId")] = this->accountId;
		break;
	case TOKEN_METHOD::EPIC_DEVICE_CODE:
		data[xorstr_("device_code")] = this->device_code;
		break;
	case TOKEN_METHOD::EPIC_EXCHANGE_CODE:
	//s	extract_key("exchange_code", &out.exchange_code);
		data[xorstr_("exchange_code")] = this->exchange_code;
		break;
	case TOKEN_METHOD::EPIC_REFRESH_TOKEN:
		//extract_key("refresh_token", &out.refresh_token);
		data[xorstr_("refresh_token")] = this->refresh_token;
		break;
	}

	return data;
}


bool platanium::authentification::Credentials::cipher(std::string& out_data)
{
	nlohmann::json data = this->to_json();

	const std::string raw = data.dump();

	DATA_BLOB raw_blob;
	raw_blob.pbData = (BYTE*)raw.c_str();
	raw_blob.cbData = (DWORD)raw.size();

	DATA_BLOB out;
	ZeroMemory(&out, sizeof(out));

	if (!CryptProtectData(&raw_blob, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &out))
	{
		spdlog::error(xorstr_("Failed to crypt data"));
		error::set_last_error(error::FAILED_TO_CIPHER_DATA);
		return false;
	}

	out_data = base64_encode(out.pbData, out.cbData, false);
	return true;
}

bool platanium::authentification::Credentials::uncipher(const std::string in, Credentials& out)
{
	std::string raw_data = base64_decode(in);

	DATA_BLOB raw_blob;
	raw_blob.pbData = (BYTE*)raw_data.c_str();
	raw_blob.cbData = (DWORD)raw_data.size();

	DATA_BLOB out_data;
	ZeroMemory(&out_data, sizeof(out_data));

	if (!CryptUnprotectData(&raw_blob, NULL, NULL, NULL, NULL, CRYPTPROTECT_LOCAL_MACHINE, &out_data))
	{
		spdlog::error(xorstr_("Failed to decrypt data"));
		error::set_last_error(error::FAILED_TO_UNCIPHER_DATA);
		throw std::runtime_error(xorstr_("Failed to decrypt data"));
	}

	std::string json_raw = std::string((char*)out_data.pbData, out_data.cbData);

	const nlohmann::json json_data = nlohmann::json::parse(json_raw);

//	spdlog::info("out json : {}", json_data.dump(4));

	return Credentials::from(json_data, out);
}