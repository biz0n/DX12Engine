#include "Application.h"

#include <View.h>
#include <RenderContext.h>
#include <SwapChain.h>
#include <Game.h>
#include <Timer.h>
#include <CommandQueue.h>
#include <UIRenderContext.h>

namespace Engine
{
    Application::Application()
    {
    }

    Application::~Application()
    {
    }

    void Application::Init(View view)
    {
        timer.Reset();

        mRenderContext = MakeShared<RenderContext>(view);

        mGame = MakeShared<Game>(mRenderContext);
        mGame->Initialize();
    }

    void Application::Destroy()
    {
        mRenderContext->GetGraphicsCommandQueue()->Flush();
    }

    void Application::Draw()
    {
        timer.Tick();

        if (timer.IsPaused())
        {
            Sleep(16);
        }
        else
        {
            mRenderContext->BeginFrame();

            mGame->Update(timer);
            mGame->Draw(timer);

            mRenderContext->EndFrame();
        }
    }

    void Application::OnActiveChanged(bool isActive)
    {
        if (isActive)
        {
            timer.Start();
        }
        else
        {
            timer.Stop();
        }
    }

    void Application::OnKeyPressed(KeyEvent event)
    {
        mGame->KeyPressed(event);
    }

    void Application::OnResize(int32 width, int32 height)
    {
        mRenderContext->GetGraphicsCommandQueue()->Flush();

        mRenderContext->GetSwapChain()->Resize(width, height);

        mGame->Resize(width, height);

        mRenderContext->GetUIContext()->Resize(width, height);
    }
} // namespace Engine