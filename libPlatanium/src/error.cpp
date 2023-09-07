#include "platanium.hpp"

void platanium::error::set_last_error(error::PLATANIUM_STATUS error)
{
	platanium::error::last_error = error;
	spdlog::debug("Last error has been set to {}", (int)error);
}

platanium::error::PLATANIUM_STATUS platanium::error::get_last_error(void)
{
	return platanium::error::last_error;
}
