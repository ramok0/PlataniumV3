#include "window.hpp"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

void Window::Close(void)
{
	this->bCloseWindow = true;
}