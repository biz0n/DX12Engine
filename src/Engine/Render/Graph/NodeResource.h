#pragma once

#include <Name.h>
#include <Types.h>


#include <d3d12.h>

namespace Engine::Render::Graph
{
    class NodeResource
    {
    public:
        Name name;
        D3D12_RESOURCE_STATES state;
        int32 subresource;
    };
}
