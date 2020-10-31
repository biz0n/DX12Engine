#pragma once

#include <Types.h>
#include <Events.h>
#include <Timer.h>

namespace Engine
{
    class RenderContext;
    class App;
    class SwapChain;
    class Keyboard;
    struct View;

    namespace Scene
    {
        class SceneObject;
        struct SceneLoadingInfo;
    }

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
        void InitScene(UniquePtr<Scene::SceneObject> scene);

    private:
        SharedPtr<RenderContext> mRenderContext;
        SharedPtr<Keyboard> mKeyboard;
        SharedPtr<Scene::SceneLoadingInfo> mSceneLoadingInfo;

        UniquePtr<Scene::SceneObject> mScene;
    private:
        Timer timer;
    };
} // namespace Engine