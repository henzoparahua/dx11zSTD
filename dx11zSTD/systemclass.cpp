#include "systemclass.h"

SystemClass::SystemClass() :
    m_Input(nullptr),
    m_Application(nullptr),
    screenWidth(0),
    screenHeight(0),
    FULL_SCREEN(false)  // Inicialmente definido como modo janela
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
    InitializeWindows();

    m_Input = new InputClass;
    m_Input->Initialize();

    m_Application = new ApplicationClass;

    bool result = m_Application->Initialize(screenWidth, screenHeight, m_hwnd);
    if (!result)
    {
        return false;
    }

    return true;
}

void SystemClass::Shutdown()
{
    if (m_Application)
    {
        m_Application->Shutdown();
        delete m_Application;
        m_Application = nullptr;
    }
    if (m_Input)
    {
        delete m_Input;
        m_Input = nullptr;
    }
    ShutdownWindows();
}

void SystemClass::Run()
{
    MSG msg;
    bool done, result;

    ZeroMemory(&msg, sizeof(MSG));

    done = false;
    while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
        {
            done = true;
        }
        else
        {
            result = Frame();
            if (!result)
            {
                done = true;
            }
        }
    }
}

bool SystemClass::Frame()
{
    bool result;

    if (m_Input->IsKeyDown(VK_ESCAPE))
    {
        return false;
    }

    if (m_Input->IsKeyDown(0x46))  // 0x46 é o código virtual da tecla F
    {
        DEVMODE dmScreenSettings;
        ZeroMemory(&dmScreenSettings, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);

        if (FULL_SCREEN)
        {
            FULL_SCREEN = false;
            screenWidth = 1278;
            screenHeight = 720;

            // Voltar para modo janela
            ChangeDisplaySettings(NULL, 0);

            // Redefinir posição da janela
            int posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
            int posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
            SetWindowPos(m_hwnd, HWND_TOP, posX, posY, screenWidth, screenHeight, SWP_SHOWWINDOW);
        }
        else
        {
            FULL_SCREEN = true;
            screenWidth = GetSystemMetrics(SM_CXSCREEN);
            screenHeight = GetSystemMetrics(SM_CYSCREEN);

            dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
            dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
            dmScreenSettings.dmBitsPerPel = 32;
            dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            LONG result = ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

            if (result == DISP_CHANGE_SUCCESSFUL)
            {
                // Se a alteração for bem sucedida, ajustar a posição da janela
                SetWindowPos(m_hwnd, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
            }
            else
            {
                FULL_SCREEN = false; // Reverter para modo janela se a mudança falhar
            }
        }
    }

    result = m_Application->Frame();

    if (!result)
    {
        return false;
    }
    return true;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    switch (umsg)
    {
    case WM_KEYDOWN:
        m_Input->KeyDown((unsigned int)wparam);
        return 0;
    case WM_KEYUP:
        m_Input->KeyUp((unsigned int)wparam);
        return 0;
    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
}

void SystemClass::InitializeWindows()
{
    WNDCLASSEX wc;
    DEVMODE dmScreenSettings;
    int posX, posY;

    ApplicationHandle = this;
    m_hinstance = GetModuleHandle(NULL);
    m_applicationName = L"Henzoparahua dx11zSTD";

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hinstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hIconSm = wc.hIcon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = m_applicationName;
    wc.cbSize = sizeof(WNDCLASSEX);

    RegisterClassEx(&wc);

    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (FULL_SCREEN)
    {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
        dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

        posX = posY = 0;
    }
    else
    {
        screenWidth = 1278;
        screenHeight = 720;

        posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }

    m_hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName,
        WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
        posX, posY, screenWidth, screenHeight, NULL, NULL, m_hinstance, NULL);

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);
    ShowCursor(false);
}

void SystemClass::ShutdownWindows()
{
    ShowCursor(true);
    if (FULL_SCREEN)
    {
        ChangeDisplaySettings(NULL, 0);
    }

    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    UnregisterClass(m_applicationName, m_hinstance);
    m_hinstance = NULL;
    ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    default:
        return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
    }
}
