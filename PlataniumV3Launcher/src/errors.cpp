#include "../include/plataniumv3launcher.hpp"

PLATANIUM_FAILURE_REASON platalog_error(PLATANIUM_FAILURE_REASON failure, std::string failedFunctionName)
{
	switch (failure)
	{
	case PLATANIUM_NO_FAILURE:
		spdlog::info("{} did not failed", failedFunctionName);
		break;
	case PLATANIUM_EXPIRED_REFRESH_TOKEN:
		spdlog::error("{} failed because the refresh token is expired", failedFunctionName);
		break;
	case PLATANIUM_FILE_DOES_NOT_EXISTS:
		spdlog::error("{} failed a file does not exists", failedFunctionName);
		break;
	case PLATANIUM_MISSING_AUTHORIZATION_CODE:
		spdlog::error("{} failed because there is no authorization code", failedFunctionName);
		break;
	case PLATANIUM_MISSING_DEVICE_AUTH:
		spdlog::error("{} failed because the device auth is missing", failedFunctionName);
		break;
	case PLATANIUM_MISSING_ACCOUNT:
		spdlog::error("{} failed because the current account is null or empty", failedFunctionName);
		break;
	case PLATANIUM_MISSING_SECRET:
		spdlog::error("{} failed because the deviceauth secret is null or empty", failedFunctionName);
		break;
	case PLATANIUM_INVALID_DEVICE_AUTH:
		spdlog::error("{} failed because the deviceauth is invalid", failedFunctionName);
		break;
	case PLATANIUM_INVALID_RESPONSE_FROM_API:
		spdlog::error("{} failed because it received an invalid response from api", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_READFILE:
		spdlog::error("{} failed because it can't read a file", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_WRITEFILE:
		spdlog::error("{} failed because it can't parse a file", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_PARSE:
		spdlog::error("{} failed because it failed to parse something", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_DECRYPT:
		spdlog::error("{} failed because it failed to decrypt deviceauth's secret", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_CRYPT:
		spdlog::error("{} failed because it failed to crypt deviceauth's secret", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_CREATE_EXCHANGE_CODE:
		spdlog::error("{} failed because it can't create an exchange code", failedFunctionName);
		break;
	case PLATANIUM_FAILED_TO_REFRESH_TOKEN:
		spdlog::error("{} failed to refresh token", failedFunctionName);
		break;
	case PLATANIUM_JSON_INVALID:
		spdlog::error("{} failed because json is invalid or empty", failedFunctionName);
		break;
	case PLATANIUM_JSON_MISSING_KEY:
		spdlog::error("{} failed because a json required key is missing", failedFunctionName);
		break;
	case PLATANIUM_OS_ERROR:
		spdlog::error("{} failed because of a os error ({})", failedFunctionName, GetLastError());
		break;
	}

	return failure;
}