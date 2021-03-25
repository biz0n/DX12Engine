#pragma once

#include <Types.h>
#include <IO/KeyEvent.h>
#include <Timer.h>

#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>

namespace Engine
{
    class Keyboard;
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
        void InitScene(Scene::Loader::SceneDto scene);

    private:
        SharedPtr<Render::RenderContext> mRenderContext;
        SharedPtr<Keyboard> mKeyboard;
        SharedPtr<Scene::SceneLoadingInfo> mSceneLoadingInfo;

        UniquePtr<Scene::SceneObject> mScene;
    private:
        Timer timer;
    };
} // namespace Engine