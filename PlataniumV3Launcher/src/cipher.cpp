#include "../include/plataniumv3launcher.hpp"

bool cipher_secret(epic_device_auth_t* device_auth, std::string& secret)
{
	if (secret.empty()) return false;

	DATA_BLOB data;
	data.pbData = (BYTE*)secret.c_str();
	data.cbData = (DWORD)secret.size();

	DATA_BLOB out{};
	ZeroMemory(&out, sizeof(out));

	if (!CryptProtectData(&data, NULL, NULL, NULL, NULL, NULL, &out))
	{
		spdlog::error("Failed to crypt 'secret' value, error: {}", GetLastError());
		return false;
	}

	std::vector<std::uint8_t> result;

	for (unsigned int i = 0; i < out.cbData; i++)
	{
		result.push_back(out.pbData[i]);
	}

	device_auth->secret = result;

	spdlog::debug("Successfully ciphered the secret deviceAuth. sizeof(device_auth->secret): {}", device_auth->secret.size());

	return true;
}

bool uncipher_secret(std::string& secret)
{
	DATA_BLOB in;
	in.cbData = (DWORD)g_configuration->deviceAuth.secret.size();
	in.pbData = (BYTE*)g_configuration->deviceAuth.secret.data();

	DATA_BLOB out{0};

	if (!CryptUnprotectData(&in, NULL, NULL, NULL, NULL, NULL, &out)) {
		spdlog::error("failed to decrypt deviceauth's secret.");
		return false;
	}

	secret = std::string((char*)out.pbData, (size_t)out.cbData);

	return true;
}