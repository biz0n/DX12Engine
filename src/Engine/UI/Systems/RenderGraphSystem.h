#pragma once

#include <Types.h>
#include <Scene/Systems/System.h>

namespace Engine::UI::Systems
{
    class RenderGraphSystem : public Scene::Systems::System
    {
        public:
            RenderGraphSystem();
            ~RenderGraphSystem() override;
        public:
            void Process(Scene::SceneObject *scene, const Timer& timer) override;
    };
}