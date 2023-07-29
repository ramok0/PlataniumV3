#include "../include/plataniumv3launcher.hpp"

bool config_exists(void)
{
	return fs::exists(fs::current_path() / "config.json");
}

PLATANIUM_FAILURE_REASON create_default_config(void)
{
	g_configuration->detourURL = false;
	g_configuration->disableSSL = false;
	g_configuration->useProxy = false;
	g_configuration->should_check_pak = false;
	g_configuration->forwardHost = "";
	g_configuration->forwardPort = 0;
	g_configuration->forwardProxy = "";
	g_configuration->dump_aes = false;
	g_configuration->debug_websockets = false;
	g_configuration->no_dll = false;
	g_configuration->use_custom_arguments = false;
	g_configuration->custom_arguments = "";
	
	fs::path outFortntePath;
	if (PLATANIUM_OK(platalog_error(fortnite_find_default_installation_path(outFortntePath), "fortnite_find_default_installation_path")))
	{
		g_configuration->fortnite_build.path = outFortntePath.string();
		platalog_error(find_fortnite_engine_version(), "find_fortnite_engine_version");
	}


	if (!PLATANIUM_OK(platalog_error(write_configuration(), "write_configuration")))
	{
		return PLATANIUM_FAILED_TO_WRITEFILE;
	}

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON read_config(void)
{
	if (!config_exists())
	{
		if (!PLATANIUM_OK(platalog_error(create_default_config(), "create_default_config")))
		{
			return PLATANIUM_FAILED_TO_WRITEFILE;
		}
	}

	fs::path config_path = fs::current_path() / "config.json";

	spdlog::trace("{} - reading config at => {}", __FUNCTION__, config_path.string());

	std::ifstream stream(config_path);

	if (!stream.is_open()) {
		return PLATANIUM_FAILED_TO_READFILE;
	}

	nlohmann::json configJson = nlohmann::json::parse(stream);

	stream.close();

	if (!PLATANIUM_OK(platalog_error(parse_configuration(configJson, g_configuration), "parse_configuration")))
	{
		return PLATANIUM_FAILED_TO_PARSE;
	}

	return PLATANIUM_NO_FAILURE;
}

PLATANIUM_FAILURE_REASON write_configuration(void)
{
	static fs::path configuration_path = fs::current_path() / "config.json";

	std::ofstream config_stream(configuration_path);

	if (!config_stream.is_open()) return PLATANIUM_FAILED_TO_READFILE;

	nlohmann::json json = {
		{"disableSSL", g_configuration->disableSSL},
		{"detourURL", g_configuration->detourURL},
		{"useProxy", g_configuration->useProxy},
		{"forwardProxy", g_configuration->forwardProxy},
		{"forwardHost", g_configuration->forwardHost},
		{"forwardPort", g_configuration->forwardPort},
		{"fortnite_path", g_configuration->fortnite_build.path},
		{"should_check_pak", g_configuration->should_check_pak},
		{"dump_aes", g_configuration->dump_aes},
		{"debug_websockets", g_configuration->debug_websockets},
		{"no_dll", g_configuration->no_dll},
		{"use_custom_arguments", g_configuration->use_custom_arguments},
		{"custom_arguments", g_configuration->custom_arguments},
	};

	if (!g_configuration->deviceAuth.account_id.empty() && !g_configuration->deviceAuth.device_id.empty() && !g_configuration->deviceAuth.secret.empty())
	{
		json["device_auth"]["account_id"] = g_configuration->deviceAuth.account_id;
		json["device_auth"]["device_id"] = g_configuration->deviceAuth.device_id;
		json["device_auth"]["secret"] = g_configuration->deviceAuth.secret;
	}

	config_stream << std::setw(4) << json << std::endl;
	
	spdlog::info("{} - Written config successfully (path:{})", __FUNCTION__, configuration_path.string());

	json.clear();
	config_stream.close();

	return PLATANIUM_NO_FAILURE;
}