#pragma once

#include <Types.h>
#include <vector>
#include <d3d12.h>

namespace Engine
{

    class CommandListContext
    {
    private:
        std::vector<ComPtr<ID3D12Resource>> mTrackedResources;

    public:
        CommandListContext() {}
        ~CommandListContext() {}

        void TrackResource(ComPtr<ID3D12Resource> resource)
        {
            mTrackedResources.emplace_back(resource);
        }

        void ClearResources()
        {
            mTrackedResources.clear();
        }
    };

} // namespace Engine