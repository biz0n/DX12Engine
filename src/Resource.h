#pragma once

#include "Types.h"
#include <d3d12.h>

class Resource
{
public:
    explicit Resource(const std::wstring &name);
    explicit Resource(ComPtr<ID3D12Resource> resource, const std::wstring &name);
    virtual ~Resource();

    virtual void Reset();

    void SetName(const std::wstring &name);
    std::wstring GetName() const { return mResourceName; }

    ComPtr<ID3D12Resource> GetD3D12Resource() const { return mResource; }
    void SetD3D12Resource(ComPtr<ID3D12Resource> resource);

    D3D12_RESOURCE_DESC GetResourceDescription() const { return mResource->GetDesc(); }

protected:
    ComPtr<ID3D12Resource> mResource;
    std::wstring mResourceName;
};