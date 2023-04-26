#pragma once

#ifndef __PLATANIUMV3GUI__
#define __PLATANIUMV3GUI__

#include "plataniumv3launcher.hpp"
#include <glfw/glfw3.h>
#include <glfw/glfw3native.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>
#include <imgui_notify.h>
#include <imguifilebrowser.h>

#include <string>
#include <format>

#pragma comment(lib, "lib/glfw/glfw3.lib")

//constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
constexpr const char* WINDOW_TITLE = "PlataniumV3 Launcher";
constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse;


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

#endif