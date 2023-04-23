#include "../include/platanium.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

bool get_config_path(std::filesystem::path& out)
{
	int argc;

	wchar_t** szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);

	if (szArglist == NULL)
	{
		MessageBoxA(0, "szArgList is NULL", "PlataniumV3", 0);
		return false;
	}

	for (int i = 1; i < argc; ++i) {
		std::wstring arg = szArglist[i];
		size_t pos = arg.find(L'=');

		if (pos != std::wstring::npos) {
			std::wstring key = arg.substr(0, pos);
			std::wstring value = arg.substr(pos + 1);

			if (key.contains(L"plataniumconfigpath"))
			{
				std::filesystem::path configPath = std::filesystem::path(value);
				std::cout << configPath.string() << std::endl;
				if (std::filesystem::exists(configPath))
				{
					LocalFree(szArglist);
					out = configPath;
					return true;
				}
			}
		}
	}

	LocalFree(szArglist);

	return false;
}

void read_config(std::filesystem::path config_path)
{
	std::ifstream stream(config_path);

	if (!stream.is_open()) return;

	nlohmann::json config = nlohmann::json::parse(stream);

	auto readData = [&config]<typename T>(std::string key, T & out) {
		out = config[key].get<T>();
	};

	readData("detourURL", configuration::detourURL);
	readData("useProxy", configuration::useProxy);
	readData("disableSSL", configuration::disableSSL);
	readData("forwardHost", configuration::forwardHost);
	readData("forwardProxy", configuration::forwardProxy);
	int forwardPort;
	readData("forwardPort", forwardPort);
	configuration::forwardPort = std::to_string(forwardPort);
}