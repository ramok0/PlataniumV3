
#ifndef __PLATANIUMV3LAUNCHER__
#define __PLATANIUMV3LAUNCHER__

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui_notify.h>
#include <imguifilebrowser.h>
#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <nlohmann/json.hpp>
#include <base64/base64.hpp>

#include <fstream>
#include <string>
#include <filesystem>
#pragma comment(lib, "lib/glfw/glfw3.lib")

namespace fs = std::filesystem;

//constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
constexpr const char* WINDOW_TITLE = "PlataniumV3 Launcher";
constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse;
constexpr const char* FORTNITE_IOS_GAME_CLIENT_ID = "3446cd72694c4a4485d81b77adbb2141";
constexpr const char* FORTNITE_IOS_GAME_CLIENT_SECRET = "9209d4a5e25a457fb9b07489d313b41a";
constexpr const char* EPIC_LAUNCHER_INSTALLED_PATH = "C:\\ProgramData\\Epic\\UnrealEngineLauncher\\LauncherInstalled.dat";
constexpr const char* FORTNITE_ITEM_ID = "4fe75bbc5a674f4f9b356b5c90567da5";


constexpr const char* EPIC_GENERATE_AUTHORIZATION_CODE_URL = "https://www.epicgames.com/id/api/redirect?clientId={}&responseType=code";
constexpr const char* EPIC_GENERATE_TOKEN_URL = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/token";
constexpr const char* EPIC_GENERATE_DEVICE_AUTH = "https://account-public-service-prod.ol.epicgames.com/account/api/public/account/{}/deviceAuth";
constexpr const char* EPIC_GENERATE_EXCHANGE_CODE = "https://account-public-service-prod.ol.epicgames.com/account/api/oauth/exchange";
//structures

struct epic_account_t {
	std::string account_id;
	std::string access_token;
	int expires_at;
	std::string refresh_token;
	int refresh_expires_at;
	std::string display_name;
};

inline epic_account_t** current_epic_account = new epic_account_t*();

struct epic_device_auth_t {
	std::string device_id;
	std::string account_id;
	std::vector<std::uint8_t> secret;
};

struct configuration_t {
	bool disableSSL = false;
	bool detourURL = false;
	bool useProxy = false;
	bool should_check_pak = false;
	std::string forwardProxy;
	std::string forwardHost;
	int forwardPort;
	std::string fortnite_path;
	epic_device_auth_t deviceAuth;
};

inline configuration_t* g_configuration = new configuration_t();

/*
window related functions
*/

//get window title
const std::string get_window_title(void);
//render epicgameslogin form
void render_epic_games_login_form(void);
//render main form
void render_main_form(void);
//init glfw and create window
bool create_window(GLFWwindow** lpWindow);
//init imgui
void initialize_imgui(GLFWwindow* window);
//init styles
void initialize_styles(void);
//draw the content
void window_loop(GLFWwindow* window);
//render the elements
void gui_render(void);
//cleanup memory & libraries
void cleanup_window(GLFWwindow* window);

/*
epic api related functions
*/

//login epic with authorizationCode
bool epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t* out);
//login epic with deviceAuth
bool epic_login_with_device_auth(epic_device_auth_t device_auth, epic_account_t* out);
//login epic with refresh_token
bool epic_login_with_refresh_token(void);
//create epic device auth
bool epic_create_device_auth(epic_device_auth_t* out);
//generates the bearer authorization header
std::string epic_generate_bearer_authorization(void);

//create basic authoirization
std::string epic_create_basic_authorization(std::string client_id, std::string client_secret);

//create exchange code
bool epic_create_exchange_code(std::string& out);

/*parsers related functions*/

//parse nlohmann::json to epic_account_t
bool parse_epic_account(nlohmann::json& document, epic_account_t* out);

//parse nlohmann::json to configuration_t
bool parse_configuration(nlohmann::json& document, configuration_t* out);

//parse nlohmann::json to epic_device_auth_t
bool parse_deviceauth(nlohmann::json& document, epic_device_auth_t* out, std::string& secret);

/*
fortnite client related functions
*/

//find fortnite installation path
bool fortnite_find_default_installation_path(fs::path& fortnite_out_path);
//start fortnite client and injects dll
bool start_fortnite_and_inject_dll(void);
//generate arguments to start fortnite
std::string generate_fortnite_start_arguments(fs::path fortnitePath, fs::path configPath, std::string exchangeCode);
//verify fortnite directory
bool verify_fortnite_directory(fs::path directory);

/*
configuration related functions
*/

//create the default config file
void create_default_config(void);
//a simple function to determine if the config file exists
bool config_exists(void);
//read and parse config to g_configuration
void read_config(void);
//write the configuration file
void write_configuration(void);

/*
cipher related functions
*/

//cipher the secret value
bool cipher_secret(epic_device_auth_t* out, std::string& secret);
//uncipher the secret value
bool uncipher_secret(std::string& secret);

#endif