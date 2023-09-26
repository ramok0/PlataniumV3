#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include <chrono>
#include "../include/window.hpp"

#undef CreateWindow

constexpr const wchar_t* CLASS_NAME = L"PlataniumV3";
constexpr DWORD WINDOW_STYLE = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME;


class Renderer {
public:
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    Renderer();
    ~Renderer();

    void NewFrame();

    bool CreateWindow();

    bool UpdateWindow();
    bool Initialize();
    void Render();

    template <class T>
    void SetWindowModel() {
        if (WindowModel) delete WindowModel;

        this->WindowModel = new T(this);
        this->next_update_time = std::chrono::system_clock::now();
        this->UpdateWindow();

        this->WindowModel->OnAppear();
    }

    bool WindowShouldClose();

    void MainLoop();

private:
    bool CreateDeviceD3D();
    void CreateRenderTarget();

    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
    Window* WindowModel;
    HWND m_window;
    std::chrono::system_clock::time_point next_update_time;
    WNDCLASSEXW wc;

    bool close_window = false;
};