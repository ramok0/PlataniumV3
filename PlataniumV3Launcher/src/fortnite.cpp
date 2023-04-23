#include "../include/plataniumv3launcher.hpp"


bool fortnite_find_default_installation_path(fs::path& fortnite_out_path)
{
	fs::path launcherInstalled = fs::path(EPIC_LAUNCHER_INSTALLED_PATH);

	if (!fs::exists(launcherInstalled)) return false;

	std::ifstream stream(launcherInstalled);

	if (!stream.is_open())
	{
		return false;
	}

	nlohmann::json data = nlohmann::json::parse(stream);
	
	if (data.find("InstallationList") == data.end())
	{
		stream.close();
		return false;
	}

	for (auto& installation : data["InstallationList"])
	{
		if (installation.find("ItemId") == installation.end()) continue;
		std::string itemId = installation["ItemId"].get<std::string>();
		if (itemId != FORTNITE_ITEM_ID) continue;

		if (installation.find("InstallLocation") == installation.end()) break;

		std::string installLocation = installation["InstallLocation"].get<std::string>();
		spdlog::info("Found Fortnite install location => {}", installLocation);
		fortnite_out_path = fs::path(installLocation);
		return true;
	}
	

	stream.close();
	return false;
}


bool verify_fortnite_directory(fs::path directory)
{
	if (!fs::exists(directory / "FortniteGame") && fs::exists(directory / "Engine")) {
		return false;
	}

	return fs::exists(directory / "FortniteGame" / "Binaries" / "Win64" / "FortniteClient-Win64-Shipping.exe");
}