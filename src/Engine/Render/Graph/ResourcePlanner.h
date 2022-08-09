#pragma once

#include <Name.h>
#include <Types.h>

#include <vector>
#include <unordered_map>
#include <d3d12.h>
#include <d3dx12.h>

namespace Engine::Render::Graph
{
    class ResourcePlanner
    {
    public:
        ResourcePlanner();
    public:
        D3D12_RESOURCE_DESC& GetResource(const Name& name, Optional<Name> originalName = std::nullopt);

    private:
        const Name& GetOriginalName(const Name& name);
    private:
        std::vector<D3D12_RESOURCE_DESC> mResources;
        std::unordered_map<Name, Name> mAliasesMap;
        std::unordered_map<Name, Index> mResourcesMap;
    };
}