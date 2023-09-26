#include "../include/platanium.h"
#include "../include/ue4.h"

CURLcode Platanium::curl_setopt_w(void* handle, CURLoption opt, ...)
{
	va_list arg;
	va_start(arg, opt);

	CURLcode result = this->curl_setopt(handle, opt, arg);

	va_end(arg);

	return result;
}

FEngineVersion* Platanium::GetEngineVersion(void)
{
	return (FEngineVersion*)this->get_engine_version();
}