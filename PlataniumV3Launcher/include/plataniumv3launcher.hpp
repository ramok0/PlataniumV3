
#ifndef __PLATANIUMV3LAUNCHER__
#define __PLATANIUMV3LAUNCHER__

#include <string.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <base64/base64.hpp>
#include <dpapi.h>
#include <shellapi.h>
#include <fstream>
#include <string>
#include <filesystem>


namespace fs = std::filesystem;

constexpr const char* FORTNITE_IOS_GAME_CLIENT_ID = "3446cd72694c4a4485d81b77adbb2141";
constexpr const char* FORTNITE_IOS_GAME_CLIENT_SECRET = "9209d4a5e25a457fb9b07489d313b41a";
constexpr const char* FORTNITE_ITEM_ID = "4fe75bbc5a674f4f9b356b5c90567da5";
constexpr const char* EPIC_LAUNCHER_INSTALLED_PATH = "C:\\ProgramData\\Epic\\UnrealEngineLauncher\\LauncherInstalled.dat";
constexpr const char* EPIC_GENERATE_AUTHORIZATION_CODE_URL = "https://www.epicgames.com/id/api/redirect?clientId={}&responseType=code";
constexpr const char* EPIC_GENERATE_TOKEN_URL = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";
constexpr const char* EPIC_GENERATE_DEVICE_AUTH = "https://account-public-service-prod.ol.epicgames.com/account/api/public/account/{}/deviceAuth";
constexpr const char* EPIC_GENERATE_EXCHANGE_CODE = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/exchange";
constexpr const char* EPIC_CALDERA_ENDPOINT = "https://caldera-service-prod.ecosec.on.epicgames.com/caldera/api/v1/launcher/racp";

//caldera providers

static std::unordered_map<std::string, std::string> caldera_providers = {
	{"EasyAntiCheat", "eac"},
	{"EasyAntiCheatEOS", "eaceos"},
	{"BattlEye", "be"}
};

//structures

struct epic_account_t {
	std::string account_id;
	std::string access_token;
	int expires_at;
	std::string refresh_token;
	int refresh_expires_at;
	std::string display_name;
};

struct epic_device_auth_t {
	std::string device_id;
	std::string account_id;
	std::string secret;
};

struct fortnite_build_t {
	std::string path;
	float engine_version;
};

struct configuration_t {
	bool no_dll = false;
	bool disableSSL = false;
	bool detourURL = false;
	bool useProxy = false;
	bool debug_websockets = false;
	bool dump_aes = false;
	bool should_check_pak = false;
	std::string forwardProxy;
	std::string forwardHost;
	int forwardPort;
	fortnite_build_t fortnite_build;
	epic_device_auth_t deviceAuth;
};

typedef enum {
	PLATANIUM_NO_FAILURE = 1,
	PLATANIUM_EXPIRED_REFRESH_TOKEN,
	PLATANIUM_FILE_DOES_NOT_EXISTS,
	PLATANIUM_MISSING_AUTHORIZATION_CODE,
	PLATANIUM_MISSING_DEVICE_AUTH,
	PLATANIUM_MISSING_ACCOUNT,
	PLATANIUM_MISSING_SECRET,
	PLATANIUM_INVALID_DEVICE_AUTH,
	PLATANIUM_INVALID_RESPONSE_FROM_API,
	PLATANIUM_FAILED_TO_READFILE,
	PLATANIUM_FAILED_TO_WRITEFILE,
	PLATANIUM_FAILED_TO_PARSE,
	PLATANIUM_FAILED_TO_DECRYPT,
	PLATANIUM_FAILED_TO_CRYPT,
	PLATANIUM_FAILED_TO_CREATE_EXCHANGE_CODE,
	PLATANIUM_FAILED_TO_GET_CALDERA,
	PLATANIUM_FAILED_TO_REFRESH_TOKEN,
	PLATANIUM_JSON_INVALID,
	PLATANIUM_JSON_MISSING_KEY,
	PLATANIUM_OS_ERROR 
} PLATANIUM_FAILURE_REASON;

#define PLATANIUM_OK(failure_reason) failure_reason == PLATANIUM_FAILURE_REASON::PLATANIUM_NO_FAILURE

#define PLATANIUM_LOG(failure_reason, funcName) PLATANIUM_OK(platalog_error(failure_reason, funcName))

PLATANIUM_FAILURE_REASON platalog_error(PLATANIUM_FAILURE_REASON failure, std::string failedFunctionName);

inline configuration_t* g_configuration = new configuration_t();
inline epic_account_t** current_epic_account = new epic_account_t * ();

/*
epic api related functions
*/

//login epic with authorizationCode
PLATANIUM_FAILURE_REASON epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t* out);
//login epic with deviceAuth
PLATANIUM_FAILURE_REASON epic_login_with_device_auth(epic_device_auth_t device_auth, epic_account_t* out);
//login epic with refresh_token
PLATANIUM_FAILURE_REASON epic_login_with_refresh_token(void);
//create epic device auth
PLATANIUM_FAILURE_REASON epic_create_device_auth(epic_device_auth_t* out);
//generates the bearer authorization header
std::string epic_generate_bearer_authorization(void);

//create basic authoirization
std::string epic_create_basic_authorization(std::string client_id, std::string client_secret);

//create exchange code
PLATANIUM_FAILURE_REASON epic_create_exchange_code(std::string& out);

//get caldera code

PLATANIUM_FAILURE_REASON epic_get_caldera(std::string account_id, std::string exchange_code, std::string& ac, std::string& caldera);

/*parsers related functions*/

//parse nlohmann::json to epic_account_t
PLATANIUM_FAILURE_REASON parse_epic_account(nlohmann::json& document, epic_account_t* out);

//parse nlohmann::json to configuration_t
PLATANIUM_FAILURE_REASON parse_configuration(nlohmann::json& document, configuration_t* out);

//parse nlohmann::json to epic_device_auth_t
PLATANIUM_FAILURE_REASON parse_deviceauth(nlohmann::json& document, epic_device_auth_t* out, std::string& secret);

/*
fortnite client related functions
*/

//find fortnite installation path
PLATANIUM_FAILURE_REASON fortnite_find_default_installation_path(fs::path& fortnite_out_path);
//start fortnite client and injects dll
PLATANIUM_FAILURE_REASON start_fortnite_and_inject_dll(void);
//generate arguments to start fortnite
std::string generate_fortnite_start_arguments(fs::path fortnitePath, fs::path configPath, std::string exchangeCode, std::string ac, std::string caldera);
//verify fortnite directory
PLATANIUM_FAILURE_REASON verify_fortnite_directory(fs::path directory);
//find fortnite path staticly
PLATANIUM_FAILURE_REASON find_fortnite_engine_version(void);

/*
configuration related functions
*/

//create the default config file
PLATANIUM_FAILURE_REASON create_default_config(void);
//a simple function to determine if the config file exists
bool config_exists(void);
//read and parse config to g_configuration
PLATANIUM_FAILURE_REASON read_config(void);
//write the configuration file
PLATANIUM_FAILURE_REASON write_configuration(void);

/*
cipher related functions
*/

//cipher the secret value
PLATANIUM_FAILURE_REASON cipher_secret(epic_device_auth_t* out, std::string& secret);
//uncipher the secret value
PLATANIUM_FAILURE_REASON uncipher_secret(std::string& secret);

#endif