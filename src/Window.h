#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Types.h"
#include "Event.h"
#include "Events.h"

class Window
{
private:
    class WindowClass
    {
    public:
        static const TCHAR *GetName() noexcept { return wndClassName; }
        static HINSTANCE GetInstance() noexcept;

    private:
        WindowClass() noexcept;
        ~WindowClass();
        WindowClass(const WindowClass &) = delete;
        WindowClass &operator=(const WindowClass &) = delete;
        static constexpr const TCHAR *wndClassName = TEXT("Direct3D12 Engine Window");
        static WindowClass wndClass;
        HINSTANCE hInst;
    };

public:
    Window(int32 width, int32 height, const TCHAR *name);
    ~Window();
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;
    Optional<int> ProcessMessages() noexcept;
    HWND GetHWnd() const { return hWnd; }

    int32 GetWidth() const { return mWidth; }
    int32 GetHeight() const { return mHeight; }

    void SetFullscreen(bool);

    Event<void(int32, int32)> OnResize;
    Event<void(bool)> OnActiveChanged;
    Event<void(KeyEvent)> OnKeyPressed;
    Event<void()> OnPaint;

private:
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

private:
    HWND hWnd;

    bool mResizing;
    bool mMinimized;
    bool mMaximized;

    int32 mWidth;
    int32 mHeight;

    bool mIsFullscreen;
    RECT mWindowRect;
};
