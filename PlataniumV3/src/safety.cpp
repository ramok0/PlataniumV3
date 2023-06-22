#include "../include/platanium.hpp"
#include <array>

bool is_supported_version(void)
{
	const std::array<float, 3> SUPPORTED_VERSIONS = { 5.1f, 5.11f, 5.2f };
	float current_version = GetEngineVersion();


	return std::find(SUPPORTED_VERSIONS.begin(), SUPPORTED_VERSIONS.end(), current_version) != SUPPORTED_VERSIONS.end();
}

void check_version(void)
{
	if (!is_supported_version())
	{
		std::stringstream ss;
		ss << "[plataniumv3] - warning : this version of the engine (" << GetEngineVersion() << ") may be unsupported" << std::endl;

		MessageBoxA(0, ss.str().c_str(), "PlataniumV3", 0);
	}
}