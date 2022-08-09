#include "RenderNode.h"

#include <Graph/Resource.h>
#include <Render/Graph/ResourcePlanner.h>

#include <d3dx12.h>

namespace Engine::Render::Graph
{
    RenderNode::RenderNode(Engine::Graph::Node&& node, SharedPtr<ResourcePlanner> planner) : mNode{node}, mPlanner { planner }
    {
    }

    void RenderNode::NewRenderTarget(const Name& name)
    {
        mNode.WriteResource({name, 0});
        auto& resourceDescription = mPlanner->GetResource(name);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        // D3D12_RESOURCE_STATE_RENDER_TARGET
    }

    void RenderNode::NewDepthStencil(const Name& name)
    {
        mNode.WriteResource({ name, 0 });
        mPlanner->GetResource(name);

        auto& resourceDescription = mPlanner->GetResource(name);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // D3D12_RESOURCE_STATE_DEPTH_WRITE
    }

    void RenderNode::NewTexture(const Name& name)
    {
        mNode.WriteResource({ name, 0 });
        auto& resourceDescription = mPlanner->GetResource(name);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        // D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    }

    void RenderNode::NewBuffer(const Name& name)
    {
        mNode.WriteResource({ name, 0 });
        mPlanner->GetResource(name);
    }

    void RenderNode::WriteRenderTarget(const Name& name, const Name& originalName)
    {
        mNode.WriteResource({ name, 0 }, originalName);

        auto& resourceDescription = mPlanner->GetResource(name, originalName);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        // D3D12_RESOURCE_STATE_RENDER_TARGET
    }

    void RenderNode::WriteDepthStencil(const Name& name, const Name& originalName)
    {
        mNode.WriteResource({ name, 0 }, originalName);

        auto& resourceDescription = mPlanner->GetResource(name, originalName);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        // D3D12_RESOURCE_STATE_DEPTH_WRITE
    }

    void RenderNode::WriteTexture(const Name& name, const Name& originalName)
    {
        mNode.WriteResource({ name, 0 }, originalName);
        auto& resourceDescription = mPlanner->GetResource(name, originalName);

        resourceDescription.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        // D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    }

    void RenderNode::WriteBuffer(const Name& name, const Name& originalName)
    {
        mNode.WriteResource({ name, 0 }, originalName);
        mPlanner->GetResource(name);

        // D3D12_RESOURCE_STATE_UNORDERED_ACCESS
    }

    void RenderNode::ReadTexture(const Name& name)
    {
        mNode.ReadResource({ name, 0 });
        auto& resourceDescription = mPlanner->GetResource(name);

        resourceDescription.Flags ^= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

        // D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_DEPTH_READ
    }

    void RenderNode::ReadBuffer(const Name& name)
    {
        mNode.ReadResource({ name, 0 });
        mPlanner->GetResource(name);
    }
}