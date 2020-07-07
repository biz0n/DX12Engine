#include "GameV2.h"

#include <RenderContext.h>
#include <Canvas.h>
#include <App.h>
#include <Game.h>

namespace Engine
{
    GameV2::GameV2(/* args */)
    {
    }

    GameV2::~GameV2()
    {
    }

    void GameV2::Init(HWND hWnd, uint32 width, uint32 height)
    {
        mRenderContext = MakeShared<RenderContext>();
        mCanvas = MakeShared<Canvas>(hWnd, width, height, mRenderContext);
        mApp = MakeUnique<App>(mRenderContext, mCanvas);

        mGame = MakeShared<Game>(mApp.get(), mRenderContext, mCanvas);
        mApp->Init(mGame);
        mGame->Initialize();
    }

    void GameV2::Destroy()
    {
    }

    void GameV2::Draw()
    {
        mApp->OnPaint();
    }

    void GameV2::OnActiveChanged(bool isActive)
    {
        mApp->OnActiveChanged(isActive);
    }

    void GameV2::OnKeyPressed(KeyEvent event)
    {
        mApp->OnKeyPressed(event);
    }

    void GameV2::OnResize(int32 width, int32 height)
    {
        mApp->OnResize(width, height);
    }
} // namespace Engine