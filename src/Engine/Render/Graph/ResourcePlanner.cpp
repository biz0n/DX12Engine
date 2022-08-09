#include "ResourcePlanner.h"

namespace Engine::Render::Graph
{
    ResourcePlanner::ResourcePlanner() = default;

    D3D12_RESOURCE_DESC& ResourcePlanner::GetResource(const Name& name, Optional<Name> originalName)
    {
        auto resourceName = name;
        auto resourceIndexIt = mResourcesMap.find(resourceName);
        if (resourceIndexIt == mResourcesMap.end() && originalName.has_value())
        {
            mAliasesMap.insert({ name, *originalName });
            resourceName = GetOriginalName(*originalName);
            resourceIndexIt = mResourcesMap.find(resourceName);
        }

        Index resourceIndex = 0;
        if (resourceIndexIt == mResourcesMap.end())
        {
            resourceIndex = mResources.size();
            mResourcesMap.insert({ resourceName, resourceIndex });
            mResources.push_back({});
        }
        else
        {
            resourceIndex = resourceIndexIt->second;
        }

        return mResources[resourceIndex];
    }


    const Name& ResourcePlanner::GetOriginalName(const Name& name)
    {
        auto originalName = name;

        auto result = mAliasesMap.find(name);
        while (result != mAliasesMap.end())
        {
            originalName = result->second;
            result = mAliasesMap.find(originalName);
        }

        return originalName;
    }
}