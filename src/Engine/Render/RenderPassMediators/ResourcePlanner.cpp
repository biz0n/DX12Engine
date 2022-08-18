#include "ResourcePlanner.h"

#include <Graph/Node.h>
#include <Graph/Resource.h>
#include <Render/FrameResourceProvider.h>

#include <functional>

namespace Engine::Render
{
    ResourcePlanner::ResourcePlanner(Engine::Graph::Node* renderNode, SharedPtr<FrameResourceProvider> frameResourceProvider)
        : mRenderNode{ renderNode }, mFrameResourcesProvider{frameResourceProvider}
    {

    }

    ResourcePlanner::~ResourcePlanner() = default;

    void ResourcePlanner::NewRenderTarget(const Name &name, const Memory::TextureCreationInfo &textureInfo)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceCreate(name, textureInfo, D3D12_RESOURCE_STATE_RENDER_TARGET, [](ScheduledResourceInfo& resourceInfo) 
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            });
    }

    void ResourcePlanner::NewDepthStencil(const Name &name, const Memory::TextureCreationInfo &textureInfo)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceCreate(name, textureInfo, D3D12_RESOURCE_STATE_DEPTH_WRITE, [](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
            });
    }

    void ResourcePlanner::NewTexture(const Name &name, const Memory::TextureCreationInfo &textureInfo)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceCreate(name, textureInfo, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, [](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            });
    }

    void ResourcePlanner::WriteRenderTarget(const Name& name, const Name& aliasName)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 }, aliasName);
        mFrameResourcesProvider->QueueResourceWrite(name, aliasName, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_RENDER_TARGET;
            });
    }

    void ResourcePlanner::WriteDepthStencil(const Name& name, const Name& aliasName)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 }, aliasName);
        mFrameResourcesProvider->QueueResourceWrite(name, aliasName, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
            });
    }

    void ResourcePlanner::WriteTexture(const Name& name, const Name& aliasName)
    {
        mRenderNode->WriteResource(Engine::Graph::Resource{ name, 0 }, aliasName);
        mFrameResourcesProvider->QueueResourceWrite(name, aliasName, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            });
    }

    void ResourcePlanner::ReadRenderTarget(const Name &name)
    {
        mRenderNode->ReadResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceRead(name, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
            });
    }

    void ResourcePlanner::ReadDeptStencil(const Name &name)
    {
        mRenderNode->ReadResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceRead(name, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_DEPTH_READ;
            });
    }

    void ResourcePlanner::ReadTexture(const Name& name)
    {
        mRenderNode->ReadResource(Engine::Graph::Resource{ name, 0 });
        mFrameResourcesProvider->QueueResourceRead(name, [&](ScheduledResourceInfo& resourceInfo)
            {
                resourceInfo.ExpectedStates |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
            });
    }
} // namespace Engine::Render