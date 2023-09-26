#include <window.hpp>
#include <iostream>
#include <renderer.h>


int main() {
	//Frontend::GetInstance()

	if (!platanium::initialize())
		return 1;

	Renderer* renderer = new Renderer();

	if (renderer->Initialize())
	{
		renderer->SetWindowModel<AuthentificationWindow>();
		renderer->MainLoop();
	}
	else {
		std::cout << "failed to init renderer" << std::endl;
	}
	

	//AuthentificationWindow* auth_window = new AuthentificationWindow();


	//auth_window->main_loop();

	//delete auth_window;

	//std::this_thread::sleep_for(std::chrono::milliseconds(550));

	//MainWindow* main_window = new MainWindow();

	//main_window->main_loop();

	//delete main_window;
	
	platanium::destroy();

	return 0;
}