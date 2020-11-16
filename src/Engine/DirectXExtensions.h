#pragma once

#include <Types.h>
#include <d3d12.h>

namespace Engine
{
    inline std::wstring GetResourceName(ID3D12Resource* resource)
    {
        wchar_t name[128] = {};
        UINT size = sizeof(name);
        resource->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);
        return name;
    }

    inline std::wstring GetResourceName(ComPtr<ID3D12Resource> resource)
    {
        return GetResourceName(resource.Get());
    }
}