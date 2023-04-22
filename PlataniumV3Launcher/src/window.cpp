#include "../include/plataniumv3launcher.hpp"
#include <karlafont.h>

bool create_window(GLFWwindow** lpWindow)
{
	if (!lpWindow)
	{
		spdlog::critical("lpWindow is nullptr, aborting."); 
		return false;
	}

	if (!glfwInit())
	{
		spdlog::critical("glfwInit returned false, aborting.");
		return false;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = *lpWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	if (window == nullptr)
	{
		spdlog::critical("failed to create window, glfwCreateWindow returned nullptr.");
		return false;
	}

	glfwMakeContextCurrent(window);

	spdlog::info("created window handle");

	glfwSwapInterval(1);

	return true;
}

void initialize_imgui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	spdlog::info("Initialized ImGui successfully");

	initialize_styles();
}

void initialize_styles(void)
{
	ImGui::StyleColorsClassic();
	ImGuiStyle style = ImGui::GetStyle();
	
	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(karla_compressed_data, karla_compressed_size, 17.f, &font_cfg);
	ImGui::MergeIconsWithLatestFont(16.f, false);
	spdlog::info("Initialized styles successfully");
}

void window_loop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		gui_render();

		ImGui::Render();
		ImGui::EndFrame(); 
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
}

void cleanup_window(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}