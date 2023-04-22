#include "../include/platanium.hpp"

void check_version(void)
{
	if (GetEngineVersion() != 5.11f)
	{
		std::stringstream ss;
		ss << "[plataniumv3] - warning : this version of the engine (" << GetEngineVersion() << ") may be unsupported" << std::endl;

		MessageBoxA(0, ss.str().c_str(), "PlataniumV3", 0);
	}
}