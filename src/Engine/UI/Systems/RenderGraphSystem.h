#pragma once

#include <Types.h>
#include <Scene/Systems/System.h>
#include <Render/RenderForwards.h>
#include <Graph/Resource.h>
#include <Graph/GraphForwards.h>
#include <Name.h>
#include <unordered_map>
#include <unordered_set>

namespace Engine::UI::Systems
{
    class RenderGraphSystem : public Scene::Systems::System
    {
        private:
            struct NodeSizes
            {
                float NodeWidth;
                float TitleWidth;
                float ReadWidth;
                float WriteWidth;
                float Spacing;
            };
        public:
            RenderGraphSystem(SharedPtr<Engine::Render::Renderer> renderer);
            ~RenderGraphSystem() override;
        public:
            void Process(Scene::SceneObject *scene, const Timer& timer) override;
        private:
            NodeSizes CalculateNodeSizes(const Engine::Graph::Node*);
        private:
            bool mShowResourcesLinks;
            bool mShowNodesLinks;
            bool mShowNodesDebugLinks;

            SharedPtr<Engine::Render::Renderer> mRenderer;
            std::unordered_map<Engine::Graph::Resource, std::unordered_set<Engine::Name>> mReads;
    };
}