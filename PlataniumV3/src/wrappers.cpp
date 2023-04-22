#include "../include/platanium.hpp"

float GetEngineVersion()
{
	FEngineVersionBase* version = (FEngineVersionBase*)native::get_engine_version();
	float engineVersion = (float)version->Major + ((float)version->Minor / 10.0f) + ((float)version->Patch / 100.f);

	return engineVersion;
}

CURLcode curl_setopt(void* handle, CURLoption opt, ...)
{
	va_list arg;
	va_start(arg, opt);
	
	CURLcode result = native::curl_setopt(handle, opt, arg);

	va_end(arg);

	return result;
}