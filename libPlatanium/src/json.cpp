#include <platanium.hpp>

bool platanium::json::json_exists(const nlohmann::json& doc, const std::string& key)
{
	return doc.find(key) != doc.end();
}