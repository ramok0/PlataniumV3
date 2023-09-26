#include "../include/renderer.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;

LRESULT __stdcall Renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

Renderer::Renderer() : device(nullptr), context(nullptr), swapChain(nullptr), renderTargetView(nullptr) {
    ZeroMemory(&this->wc, sizeof(this->wc));

    this->next_update_time = std::chrono::system_clock::now();
    this->WindowModel = nullptr;
    this->m_window = NULL;
}

Renderer::~Renderer() {
    if (WindowModel) delete WindowModel;

    ::UnregisterClassW(this->wc.lpszClassName, this->wc.hInstance);

    if (renderTargetView) renderTargetView->Release();
    if (swapChain) swapChain->Release();
    if (context) context->Release();
    if (device) device->Release();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::NewFrame()
{
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            close_window = true;
    }

    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        if (renderTargetView) renderTargetView->Release();
        swapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

bool Renderer::CreateWindow()
{
    this->wc.lpfnWndProc = WndProc;
    this->wc.style = 0;
    this->wc.lpszClassName = CLASS_NAME;
    this->wc.hInstance = GetModuleHandleA(0);
    this->wc.cbSize = sizeof(WNDCLASSEXW);

    RegisterClassExW(&this->wc);

    HWND wnd = CreateWindowExW(0L, CLASS_NAME, L"Uninitialized Platanium Window", WINDOW_STYLE, 100, 100, 1000, 1000, 0, 0, 0, 0);
    ::ShowWindow(wnd, SW_SHOW);
    ::UpdateWindow(wnd);

    this->m_window = wnd;

    return wnd != 0;
}

void Renderer::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
    pBackBuffer->Release();
}


bool Renderer::CreateDeviceD3D()
{
    std::cout << "Window : " << std::hex << this->m_window << std::endl;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = this->m_window;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &swapChain, &device, &featureLevel, &context);
    if (res != S_OK)
    {
        std::cerr << "D3D11CreateDeviceAndSwapChain err : " << std::hex << res << std::endl;
        return false;
    }

    CreateRenderTarget();
    return true;
}

bool Renderer::UpdateWindow()
{
    using namespace std::chrono;

    time_point now = system_clock::now();

    if (now < this->next_update_time) {
        return false;
    }

    if (this->WindowModel)
    {
        SetWindowPos(this->m_window, NULL, 0, 0, this->WindowModel->get_width(), this->WindowModel->get_height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        SetWindowTextW(this->m_window, this->WindowModel->get_title());
    }

    this->next_update_time = now + std::chrono::seconds(10);
    return true;
}

bool Renderer::Initialize() {
    if (!this->CreateWindow()) {
        std::cerr << "failed to create window" << std::endl;
        return false;
    }
    if (!this->CreateDeviceD3D()) {
        std::cerr << "failed to create device" << std::endl;
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(this->m_window);
    ImGui_ImplDX11_Init(device, context);


    return true;
}

void Renderer::Render() {
    ImGui::Render();
    float clear_color_with_alpha[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    context->OMSetRenderTargets(1, &renderTargetView, nullptr);
    context->ClearRenderTargetView(renderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapChain->Present(1, 0);
}

bool Renderer::WindowShouldClose()
{
    return this->close_window;
}

void Renderer::MainLoop()
{
    while (!WindowShouldClose())
    {
        this->UpdateWindow();

        this->NewFrame();

        if (this->WindowModel)
        {
            this->WindowModel->render();
        }

        this->Render();
    }
}

