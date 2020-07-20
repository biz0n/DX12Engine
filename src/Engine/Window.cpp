#include "Window.h"

#include <GameV2.h>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <iostream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine
{
    Window::WindowClass Window::WindowClass::wndClass;

    Window::WindowClass::WindowClass() noexcept : hInst(GetModuleHandle(nullptr))
    {
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        WNDCLASS wnd;
        wnd.style = CS_HREDRAW | CS_VREDRAW;
        wnd.lpfnWndProc = HandleMsgSetup;
        wnd.cbClsExtra = 0;
        wnd.cbWndExtra = 0;
        wnd.hInstance = GetInstance();
        wnd.hIcon = LoadIcon(0, IDI_APPLICATION);
        wnd.hCursor = LoadCursor(0, IDC_ARROW);
        wnd.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        wnd.lpszMenuName = 0;
        wnd.lpszClassName = GetName();

        if (!RegisterClass(&wnd))
        {
            MessageBox(0, TEXT("RegisterClass Failed."), 0, 0);
        }
    }

    Window::WindowClass::~WindowClass()
    {
        UnregisterClass(wndClassName, GetInstance());
    }

    HINSTANCE Window::WindowClass::GetInstance() noexcept
    {
        return wndClass.hInst;
    }

    Window::Window(int32 width, int32 height, const TCHAR *name)
        : mWidth(width), mHeight(height), mName(name), mIsFullscreen(false), hWnd(0)
    {
    }

    Window::~Window()
    {
        DestroyWindow(hWnd);
    }

    int Window::Run(GameV2* game)
    {
        bool isGameInitialized = false;
        try
        {
            mGame = game;

            RECT R = {0, 0, mWidth, mHeight};
            AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

            hWnd = CreateWindow(
                WindowClass::GetName(),
                mName,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                R.right - R.left,
                R.bottom - R.top,
                0,
                0,
                WindowClass::GetInstance(),
                this);

            if (hWnd == nullptr)
            {
                MessageBox(0, TEXT("RegisterClass Failed."), 0, 0);
            }

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hWnd);

            isGameInitialized = true;
            game->Init(hWnd, mWidth, mHeight);

            ShowWindow(hWnd, SW_SHOW);
            UpdateWindow(hWnd);

            MSG msg = {};
            while (msg.message != WM_QUIT)
            {
                // Process any messages in the queue.
                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            game->Destroy();

            return static_cast<int>(msg.wParam);
        }
        catch (std::exception &e)
        {
            MessageBox(nullptr, e.what(), TEXT("Application hit a problem"), MB_OK);

            if (isGameInitialized)
            {
                game->Destroy();
            }
            return EXIT_FAILURE;
        }
    }

    LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
        if (msg == WM_NCCREATE)
        {
            // extract ptr to window class from creation data
            const CREATESTRUCTW *const pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
            Window *const pWnd = static_cast<Window *>(pCreate->lpCreateParams);
            // set WinAPI-managed user data to store ptr to window instance
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
            // set message proc to normal (non-setup) handler now that setup is finished
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
            // forward message to window instance handler
            return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
        }
        // if we get a message before the WM_NCCREATE message, handle with default handler
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        // retrieve ptr to window instance
        Window *const payload = reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        // forward message to window instance handler
        return payload->HandleMsg(hWnd, msg, wParam, lParam);
    }

    LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        Window *const payload = reinterpret_cast<Window *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        GameV2* game = payload->mGame;

        switch (msg)
        {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

        case WM_ACTIVATE:
        {
            game->OnActiveChanged(LOWORD(wParam) != WA_INACTIVE);
            return 0;
        }

        case WM_SIZE:
            mWidth = LOWORD(lParam);
            mHeight = HIWORD(lParam);
            if (wParam == SIZE_MINIMIZED)
            {
                mMinimized = true;
                mMaximized = false;
                game->OnActiveChanged(false);
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                mMinimized = false;
                mMaximized = true;
                game->OnResize(mWidth, mHeight);
                game->OnActiveChanged(true);
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (mMinimized)
                {
                    mMinimized = false;
                    game->OnResize(mWidth, mHeight);
                    game->OnActiveChanged(true);
                }
                else if (mMaximized)
                {
                    mMaximized = false;
                    game->OnResize(mWidth, mHeight);
                    game->OnActiveChanged(true);
                }
                else if (mResizing)
                {
                    // do nothing
                }
                else
                {
                    game->OnResize(mWidth, mHeight);
                }
            }

            return 0;

        case WM_ENTERSIZEMOVE:
            mResizing = true;
            game->OnActiveChanged(false);
            return 0;

        case WM_EXITSIZEMOVE:
            mResizing = false;
            game->OnResize(mWidth, mHeight);
            game->OnActiveChanged(true);
            return 0;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 400;
            ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 400;
            return 0;

        case WM_MENUCHAR:
            // Don't beep when we alt-enter.
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        {
            KeyCode::Key key = (KeyCode::Key)wParam;
            KeyEvent event(key, KeyEvent::KeyState::Pressed);
            game->OnKeyPressed(event);
            return 0;
        }
        case WM_SYSKEYUP:
        case WM_KEYUP:
        {
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            else if (wParam == VK_F11)
            {
                SetFullscreen(!mIsFullscreen);
            }
            KeyCode::Key key = (KeyCode::Key)wParam;
            KeyEvent event(key, KeyEvent::KeyState::Released);
            game->OnKeyPressed(event);
            return 0;
        }
        case WM_CHAR:
            return 0;
        case WM_PAINT:
            game->Draw();
            return 0;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    void Window::SetFullscreen(bool fullscreen)
    {
        if (mIsFullscreen != fullscreen)
        {
            mIsFullscreen = fullscreen;

            if (mIsFullscreen)
            {
                ::GetWindowRect(hWnd, &mWindowRect);

                UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);

                ::SetWindowLongA(hWnd, GWL_STYLE, windowStyle);

                HMONITOR monitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFOEX monitorInfo = {};
                monitorInfo.cbSize = sizeof(MONITORINFOEX);
                ::GetMonitorInfo(monitor, &monitorInfo);

                ::SetWindowPos(hWnd, HWND_TOP,
                               monitorInfo.rcMonitor.left,
                               monitorInfo.rcMonitor.top,
                               monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                               monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                               SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(hWnd, SW_MAXIMIZE);
            }
            else
            {
                ::SetWindowLong(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

                ::SetWindowPos(hWnd, HWND_NOTOPMOST,
                               mWindowRect.left,
                               mWindowRect.top,
                               mWindowRect.right - mWindowRect.left,
                               mWindowRect.bottom - mWindowRect.top,
                               SWP_FRAMECHANGED | SWP_NOACTIVATE);

                ::ShowWindow(hWnd, SW_NORMAL);
            }
        }
    }

    Optional<int> Window::ProcessMessages() noexcept
    {
        MSG msg = {0};

        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return (int)msg.wParam;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        return {};
    }

} // namespace Engine