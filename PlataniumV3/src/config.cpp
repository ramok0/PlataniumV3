#include "../include/platanium.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <regex>

bool get_config_path(std::filesystem::path& out)
{
	char* data = GetCommandLineA();

	std::regex pattern(R"(-plataniumconfigpath=\"([^\"]+)\")");
	std::smatch match;
	std::string datastr = std::string(data);

	if (std::regex_search(datastr, match, pattern))
	{
		if (std::filesystem::exists(match[1].str()))
		{
			out = match[1].str();
			return true;
		}
	}

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

	readData("should_check_pak", configuration::bypass_pak_checks);
	readData("detourURL", configuration::detourURL);
	readData("useProxy", configuration::useProxy);
	readData("disableSSL", configuration::disableSSL);
	readData("debug_websockets", configuration::debug_websockets);
	readData("forwardHost", configuration::forwardHost);
	readData("forwardProxy", configuration::forwardProxy);
	readData("dump_aes", configuration::dump_aes);
	int forwardPort;
	readData("forwardPort", forwardPort);
	configuration::forwardPort = std::to_string(forwardPort);
}