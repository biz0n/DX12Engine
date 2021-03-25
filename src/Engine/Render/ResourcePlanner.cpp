#include "ResourcePlanner.h"

namespace Engine::Render
{
    ResourcePlanner::ResourcePlanner() = default;
    ResourcePlanner::~ResourcePlanner() = default;

    void ResourcePlanner::NewRenderTarget(const Name &name, const TextureCreationInfo &textureInfo)
    {
        mResourcesForCreate.push_back({name, textureInfo, D3D12_RESOURCE_STATE_RENDER_TARGET});
    }

    void ResourcePlanner::NewDepthStencil(const Name &name, const TextureCreationInfo &textureInfo)
    {
        mResourcesForCreate.push_back({name, textureInfo, D3D12_RESOURCE_STATE_DEPTH_WRITE});
    }

    void ResourcePlanner::ReadRenderTarget(const Name &name)
    {

    }

    void ResourcePlanner::ReadDeptStencil(const Name &name)
    {

    }
} // namespace Engine::Render