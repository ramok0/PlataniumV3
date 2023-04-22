#include "../include/plataniumv3launcher.hpp"

int main(void)
{
	GLFWwindow* window;
	if (!create_window(&window))
	{
		spdlog::info("Closing app.");
		return 1;
	}

	initialize_imgui(window);
	window_loop(window);
	cleanup_window(window);

	return 0;
}