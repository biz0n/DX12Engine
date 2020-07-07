#pragma once

#include <Types.h>
#include <Events.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Engine
{
    class RenderContext;
    class App;
    class Canvas;
    class Game;

    class GameV2
    {
    private:
        /* data */
    public:
        GameV2(/* args */);
        ~GameV2();

        void Init(HWND hWnd, uint32 width, uint32 height);
        void Destroy();

        void Draw();

        void OnActiveChanged(bool isActive);
        void OnKeyPressed(KeyEvent event);
        void OnResize(int32 width, int32 height);

    private:
        SharedPtr<RenderContext> mRenderContext;
        SharedPtr<Canvas> mCanvas;
        UniquePtr<App> mApp;
        SharedPtr<Game> mGame;
    };
} // namespace Engine