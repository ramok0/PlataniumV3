#include "../include/plataniumv3launcher.hpp"
#include <karlafont.h>

const std::string get_window_title(void)
{
	if (!current_epic_account) {
		return "Connect to Epic Games";
	}

	return "Platanium V3";
}

bool create_window(GLFWwindow** lpWindow)
{
	if (!lpWindow)
	{
		spdlog::critical("{} - lpWindow is nullptr, aborting.", __FUNCTION__);
		return false;
	}

	if (!glfwInit())
	{
		spdlog::critical("{} - glfwInit returned false, aborting.", __FUNCTION__);
		return false;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = *lpWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	if (window == nullptr)
	{
		spdlog::critical("{} - failed to create window, glfwCreateWindow returned nullptr.", __FUNCTION__);
		return false;
	}

	glfwMakeContextCurrent(window);

	spdlog::info("{} - created window handle", __FUNCTION__);

	glfwSwapInterval(1);

	return true;
}

void initialize_imgui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	spdlog::info("{} - Initialized ImGui successfully", __FUNCTION__);

	initialize_styles();
}

void initialize_styles(void)
{
	ImGui::StyleColorsClassic();
	
	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(karla_compressed_data, karla_compressed_size, 16.f, &font_cfg);
	ImGui::MergeIconsWithLatestFont(16.f, false);
	
	ImGui::GetStyle().WindowTitleAlign = { 0.5f,0.5f };
	ImGui::GetStyle().FrameRounding = 5.f;
		
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = { 0.094f, 0.094f,0.094f,1.f };
	spdlog::info("{} - Initialized styles successfully", __FUNCTION__);
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
		glClearColor(0.19f, 0.19f, 0.19f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}
}

void cleanup_window(GLFWwindow* window)
{
	delete* current_epic_account;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	spdlog::info("{} - unloaded library successfully", __FUNCTION__);
}