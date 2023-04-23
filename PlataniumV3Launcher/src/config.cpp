#include "../include/plataniumv3launcher.hpp"

bool config_exists(void)
{
	return fs::exists(fs::current_path() / "config.json");
}

void create_default_config(void)
{
	spdlog::info("creating default configuration file");

	g_configuration->detourURL = false;
	g_configuration->disableSSL = false;
	g_configuration->useProxy = false;
	g_configuration->should_check_pak = false;
	g_configuration->forwardHost = "";
	g_configuration->forwardPort = 0;
	g_configuration->forwardProxy = "";
	
	fs::path outFortntePath;
	if (fortnite_find_default_installation_path(outFortntePath))
	{
		g_configuration->fortnite_path = outFortntePath.string();
	}

	write_configuration();
}

void read_config(void)
{
	if (!config_exists())
	{
		create_default_config();
	}

	std::ifstream stream(fs::current_path() / "config.json");

	if (!stream.is_open()) {
		spdlog::critical("Failed to read configuration file !");
		throw std::runtime_error("Failed to read configuration !");
	}

	nlohmann::json configJson = nlohmann::json::parse(stream);

	stream.close();

	if (!parse_configuration(configJson, g_configuration))
	{
		spdlog::critical("Failed to read configuration file !");
		throw std::runtime_error("Failed to read configuration !");
	}

	spdlog::info("read config file successfully");
}

void write_configuration(void)
{
	static fs::path configuration_path = fs::current_path() / "config.json";

	std::ofstream config_stream(configuration_path);

	if (!config_stream.is_open()) return;

	nlohmann::json json = {
		{"disableSSL", g_configuration->disableSSL},
		{"detourURL", g_configuration->detourURL},
		{"useProxy", g_configuration->useProxy},
		{"forwardProxy", g_configuration->forwardProxy},
		{"forwardHost", g_configuration->forwardHost},
		{"forwardPort", g_configuration->forwardPort},
		{"fortnite_path", g_configuration->fortnite_path},
		{"should_check_pak", g_configuration->should_check_pak}
	};

	if (!g_configuration->deviceAuth.account_id.empty() && !g_configuration->deviceAuth.device_id.empty() && !g_configuration->deviceAuth.secret.empty())
	{
		json["device_auth"]["account_id"] = g_configuration->deviceAuth.account_id;
		json["device_auth"]["device_id"] = g_configuration->deviceAuth.device_id;
		json["device_auth"]["secret"] = g_configuration->deviceAuth.secret;
	}

	config_stream << std::setw(4) << json << std::endl;
	
	spdlog::info("Written config successfully (path:{})", configuration_path.string());

	json.clear();
	config_stream.close();
}