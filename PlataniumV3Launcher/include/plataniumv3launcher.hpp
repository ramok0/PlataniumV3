#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui_notify.h>
#include <imguifilebrowser.h>
#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <nlohmann/json.hpp>

#include <string>
#pragma comment(lib, "lib/glfw/glfw3.lib")

constexpr const int WINDOW_WIDTH = 800;
constexpr const int WINDOW_HEIGHT = 600;
constexpr const char* WINDOW_TITLE = "PlataniumV3 Launcher";
constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

//structures

struct epic_account_t {
	std::string account_id;
	std::string access_token;
	std::string refresh_token;
	int expires_at;
	int created_at;
};

struct epic_device_auth_t {
	std::string device_id;
	std::string account_id;
	std::string secret;
};

struct config_file_t {
	bool disableSSL = false;
	bool detourURL = false;
	bool useProxy = false;
	std::string forwardProxy;
	std::string forwardHost;
	std::string forwardPort;
};

/*
window related functions
*/

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
bool epic_login_with_authorization_code(std::string& authorizationCode, epic_account_t& out);

//login epic with deviceAuth
bool epic_login_with_device_auth(epic_device_auth_t& device_auth, epic_account_t& out);



//parse nlohmann::json to epic_account_t
bool parse_epic_account(nlohmann::json& document, epic_account_t& out);