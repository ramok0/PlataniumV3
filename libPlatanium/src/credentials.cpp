#include <platanium.hpp>
#include <dpapi.h>

bool platanium::authentification::Credentials::from(const nlohmann::json& in, Credentials& out)
{
	if (!json::extract_json(in, "client_id", &out.client_id) ||
		!json::extract_json(in, "refresh_token", &out.refresh_token) ||
		!json::extract_json(in, "type", &out.type))
	{
		spdlog::error("failed to parse credentials from json");
		return false;
	}

	return true;
}

nlohmann::json platanium::authentification::Credentials::to_json(void)
{
	nlohmann::json data = {
		{"client_id", this->client_id},
		{"refresh_token", this->refresh_token},
		{"type", this->type}
	};

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
		spdlog::error("Failed to crypt data");
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
		spdlog::error("Failed to decrypt data");
		error::set_last_error(error::FAILED_TO_UNCIPHER_DATA);
		throw std::runtime_error("Failed to decrypt data");
	}

	std::string json_raw = std::string((char*)out_data.pbData, out_data.cbData);

	const nlohmann::json json_data = nlohmann::json::parse(json_raw);

	spdlog::info("out json : {}", json_data.dump(4));

	return Credentials::from(json_data, out);
}