#include "../include/plataniumv3launcher.hpp"

PLATANIUM_FAILURE_REASON cipher_secret(epic_device_auth_t* device_auth, std::string& secret)
{
	DATA_BLOB data;
	data.pbData = (BYTE*)secret.c_str();
	data.cbData = (DWORD)secret.size();

	DATA_BLOB out{};
	ZeroMemory(&out, sizeof(out));

	if (!CryptProtectData(&data, NULL, NULL, NULL, NULL, NULL, &out))
	{
		spdlog::error("Failed to crypt 'secret' value, error: {}", GetLastError());
		return PLATANIUM_FAILED_TO_CRYPT;
	}

	std::vector<std::uint8_t> result;

	for (unsigned int i = 0; i < out.cbData; i++)
	{
		result.push_back(out.pbData[i]);
	}

	std::string toEncode = std::string((char*)result.data(), result.size());

	device_auth->secret = base64_encode(toEncode);

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON uncipher_secret(std::string& secret)
{
	if (g_configuration->deviceAuth.secret.empty()) return PLATANIUM_FAILURE_REASON::PLATANIUM_MISSING_SECRET;

	//passer le base64 en char*
	std::string decoded_string = base64_decode(g_configuration->deviceAuth.secret);

	DATA_BLOB in;
	in.cbData = (DWORD)decoded_string.size();
	in.pbData = (BYTE*)decoded_string.data();

	DATA_BLOB out{0};

	if (!CryptUnprotectData(&in, NULL, NULL, NULL, NULL, NULL, &out)) {
		spdlog::error("failed to decrypt deviceauth's secret. {}", GetLastError());
		return PLATANIUM_FAILURE_REASON::PLATANIUM_FAILED_TO_CRYPT;
	}

	secret = std::string((char*)out.pbData, (size_t)out.cbData);

	return PLATANIUM_FAILURE_REASON::PLATANIUM_NO_FAILURE;
}