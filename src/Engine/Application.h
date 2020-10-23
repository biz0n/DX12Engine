#pragma once

#include <Types.h>
#include <Events.h>
#include <Timer.h>

namespace Engine
{
    class RenderContext;
    class App;
    class SwapChain;
    class Game;
    struct View;

    class Application
    {
    public:
        Application();
        ~Application();

        void Init(View view);
        void Destroy();

        void Draw();

        void OnActiveChanged(bool isActive);
        void OnKeyPressed(KeyEvent event);
        void OnResize(int32 width, int32 height);

    private:
        SharedPtr<RenderContext> mRenderContext;
        SharedPtr<Game> mGame;
    private:
        Timer timer;
    };
} // namespace Engine