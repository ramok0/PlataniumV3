#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui_notify.h>
#include <spdlog/spdlog.h>
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#pragma comment(lib, "lib/glfw/glfw3.lib")

constexpr const int WINDOW_WIDTH = 800;
constexpr const int WINDOW_HEIGHT = 600;
constexpr const char* WINDOW_TITLE = "PlataniumV3 Launcher";
constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;


//window related functions

bool create_window(GLFWwindow** lpWindow);
void initialize_imgui(GLFWwindow* window);
void initialize_styles(void);
void window_loop(GLFWwindow* window);
void gui_render(void);
void cleanup_window(GLFWwindow* window);

