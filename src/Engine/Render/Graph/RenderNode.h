#pragma once

#include <Graph/Node.h>

#include <Types.h>
#include <Name.h>

namespace Engine::Render::Graph
{
    class ResourcePlanner;

    class RenderNode
    {
    public:
        RenderNode(Engine::Graph::Node&& node, SharedPtr<ResourcePlanner> planner);
    public:
        void NewRenderTarget(const Name& name);
        void NewDepthStencil(const Name& name);
        void NewTexture(const Name& name);
        void NewBuffer(const Name& name);

        void WriteRenderTarget(const Name& name, const Name& originalName);
        void WriteDepthStencil(const Name& name, const Name& originalName);
        void WriteTexture(const Name& name, const Name& originalName);
        void WriteBuffer(const Name& name, const Name& originalName);

        void ReadTexture(const Name& name);
        void ReadBuffer(const Name& name);
        
    private:
        SharedPtr<ResourcePlanner> mPlanner;
        Engine::Graph::Node mNode;
    };
}