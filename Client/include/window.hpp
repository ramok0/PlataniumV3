#pragma once
#include <platanium.hpp>
#include <string>
#include <iostream>
#include <optional>

class Renderer;

class Window {
public:
	Window(const std::wstring window_name, unsigned int Width, unsigned int Height, Renderer* renderer)
	{
		this->bCloseWindow = false;
		this->m_renderer = renderer;

		this->width = Width;
		this->height = Height;
		this->window_name = window_name;
	}

	~Window() {

	}

	virtual void OnAppear() { };

	const char* get_title_ansi() const { 
		std::string result = std::string(this->window_name.begin(), this->window_name.end());

		return result.c_str();
	};

	const wchar_t* get_title() const { return this->window_name.c_str(); };
	const size_t get_title_length() const { return this->window_name.size(); };

	const unsigned int get_width() const { return width; };
	const unsigned int get_height() const { return height; };

	virtual void render() { };

	void Close(void);
protected:
	unsigned int width;
	unsigned int height;
	std::wstring window_name;
	bool bCloseWindow;
	Renderer* m_renderer;
};

class AuthentificationWindow : public Window {
public:
	AuthentificationWindow(Renderer* renderer) : Window(L"Authentification", 800, 600, renderer) {
		this->m_RememberMe = true;
		this->m_Credentials = std::nullopt;
	};

	void render() override;

	void OnAppear() override;

private:
	bool m_RememberMe;
	std::optional<platanium::epic::api::account::authentfication::DeviceCode> m_Credentials;
	char current_code[10];
	bool bAddNewAccount;
	std::string selectedAccount;
	bool bIsSelectingNewAccount;
};

class MainWindow : public Window {
public:
	MainWindow(Renderer* renderer) : Window(L"PlataniumV3", 800, 600, renderer) {
		
	}

	void render() override;

	void OnAppear() override;
};