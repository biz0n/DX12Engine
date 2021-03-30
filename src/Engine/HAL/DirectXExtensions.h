#pragma once

#include <Types.h>
#include <d3d12.h>

namespace Engine::HAL
{
    inline std::wstring GetResourceNameW(ID3D12Object* resource)
    {
        wchar_t name[128] = {};
        UINT size = sizeof(name);
        resource->GetPrivateData(WKPDID_D3DDebugObjectNameW, &size, name);
        return name;
    }

    inline std::wstring GetResourceNameW(ComPtr<ID3D12Object> resource)
    {
        return GetResourceNameW(resource.Get());
    }

    inline std::string GetResourceName(ID3D12Object* resource)
    {
        char name[128] = {};
        UINT size = sizeof(name);
        resource->GetPrivateData(WKPDID_D3DDebugObjectName, &size, name);
        return name;
    }

    inline std::string GetResourceName(ComPtr<ID3D12Object> resource)
    {
        return GetResourceName(resource.Get());
    }

    inline void SetResourceName(ID3D12Object* resource, const std::string& name)
    {
        resource->SetPrivateData(WKPDID_D3DDebugObjectName, name.length(), name.data());
    }

    inline void SetResourceName(ComPtr<ID3D12Object> resource, const std::string& name)
    {
        SetResourceName(resource.Get(), name);
    }
}