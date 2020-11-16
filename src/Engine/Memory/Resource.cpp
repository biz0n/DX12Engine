#include "Resource.h"

namespace Engine::Memory
{
    Resource::Resource(const std::wstring &name) : mResource(nullptr)
    {
        mResourceName = name;
    }

    Resource::Resource(ComPtr<ID3D12Resource> resource, const std::wstring &name)
        : mResource(resource)
    {
        mResourceName = name;
        mResource->SetName(name.c_str());
    }

    Resource::~Resource()
    {
    }

    void Resource::SetD3D12Resource(ComPtr<ID3D12Resource> resource)
    {
        mResource = resource;
        mResource->SetName(mResourceName.c_str());
    }

    void Resource::Reset()
    {
        mResource.Reset();
    }

    void Resource::SetName(const std::wstring &name)
    {
        if (mResource)
        {
            mResource->SetName(name.c_str());
        }
        mResourceName = name;
    }

} // namespace Engine::Memory