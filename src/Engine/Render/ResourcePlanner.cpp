#include "ResourcePlanner.h"

namespace Engine::Render
{
    ResourcePlanner::ResourcePlanner() = default;
    ResourcePlanner::~ResourcePlanner() = default;

    void ResourcePlanner::NewRenderTarget(const Name &name, const TextureCreationInfo &textureInfo)
    {
        mResourcesForCreate.push_back({name, textureInfo});
    }

    void ResourcePlanner::NewDepthStencil(const Name &name, const TextureCreationInfo &textureInfo)
    {
        mResourcesForCreate.push_back({name, textureInfo});
    }

    void ResourcePlanner::ReadRenderTarget(const Name &name)
    {

    }

    void ResourcePlanner::ReadDeptStencil(const Name &name)
    {

    }
} // namespace Engine::Render