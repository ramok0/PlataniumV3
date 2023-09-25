#include <platanium.hpp>
#include <fstream>

platanium::epic::LauncherInstalled::LauncherInstalledEntry platanium::epic::LauncherInstalled::LauncherInstalledEntry::from(const nlohmann::json& doc)
{
	platanium::epic::LauncherInstalled::LauncherInstalledEntry out;

	if (!json::extract_json(doc, xorstr_("AppName"), &out.AppName)
		|| !json::extract_json(doc, xorstr_("AppVersion"), &out.AppVersion)
		|| !json::extract_json(doc, xorstr_("ArtifactId"), &out.ArtifactId)
		|| !json::extract_json(doc, xorstr_("NamespaceId"), &out.NamespaceId)
		|| !json::extract_json(doc, xorstr_("ItemId"), &out.ItemId)
		|| !json::extract_json(doc, xorstr_("InstallLocation"), &out.InstallLocation))
	{
		error::set_last_error(error::MISSING_ENTRY);
		spdlog::error(xorstr_("An entry is missing from json to parse LauncherInstalledEntry"));
		throw std::runtime_error(xorstr_("JSON MISSING ENTRY"));
	}

	return out;
}

bool platanium::epic::LauncherInstalled::exists(void)
{
	return std::filesystem::exists(this->m_path);
}

bool platanium::epic::LauncherInstalled::read(std::string& out)
{
	if (!this->exists())
	{
		error::set_last_error(error::NO_EXISTS);
		spdlog::error(xorstr_("failed to read LauncherInstalled.dat because the file does not exists"));
		return false;
	}

	std::ifstream stream(this->m_path);

	if (!stream.is_open())
	{
		error::set_last_error(error::FAILED_TO_READ_FILE);
		spdlog::error(xorstr_("failed to read LauncherInstalled.dat"));
		return false;
	}

	std::string buffer;
	while (std::getline(stream, buffer))
	{
		out += buffer;
	}

	return true;
}

bool platanium::epic::LauncherInstalled::parse(void) {
	std::string buffer;

	if (!this->read(buffer)) return false;

	nlohmann::json doc;

	try {
		doc = nlohmann::json::parse(buffer);
	}
	catch (const nlohmann::json::parse_error&) {
			set_last_error(platanium::error::JSON_PARSING_ERROR);
			//spdlog::error(xorstr_("failed to json parse launcherinstalled.dat"));
			return false;
	}

	if (!json::json_exists(doc, xorstr_("InstallationList")))
	{
		set_last_error(platanium::error::MISSING_ENTRY);
		spdlog::error("failed to find InstallationList");
		return false;
	}

	for (auto& entry_json : doc[xorstr_("InstallationList")])
	{
		auto entry = platanium::epic::LauncherInstalled::LauncherInstalledEntry::from(entry_json);

		this->m_data.push_back(entry);
	}

	spdlog::info("Parsed {} entries of InstallationList", this->m_data.size());

	return true;
}

bool platanium::epic::LauncherInstalled::find(LauncherInstalledEntry& out, std::function<bool(const LauncherInstalledEntry&)> prediction)
{
	auto it = std::find_if(this->m_data.begin(), this->m_data.end(), prediction);
	if (it == this->m_data.end()) return false;

	out = *it;
	return true;
}

bool platanium::epic::LauncherInstalled::find_by_app_name(const std::string& app_name, LauncherInstalledEntry& out)
{
	return find(out, [&app_name](const LauncherInstalledEntry& entry) {
		return entry.AppName == app_name;
		});
}

bool platanium::epic::LauncherInstalled::find_fortnite_path(std::filesystem::path& out)
{
	LauncherInstalled launcherInstalled;
	LauncherInstalledEntry entry;
	if (!launcherInstalled.find_by_app_name(xorstr_("Fortnite"), entry))
	{
		spdlog::error(xorstr_("Failed to find fortnite installation path"));
		return false;
	}

	out = std::filesystem::path(entry.InstallLocation);
	return true;
}
