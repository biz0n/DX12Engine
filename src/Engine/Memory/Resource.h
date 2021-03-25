#pragma once

#include <Types.h>
#include <d3d12.h>

#include <Memory/ResourceCopyManager.h>

namespace Engine::Memory
{
    class Resource
    {
    public:
        Resource();
        virtual ~Resource();

        void SetName(const std::string &name);
        std::string GetName() const { return mResourceName; }

        ComPtr<ID3D12Resource> D3DResourceCom() const { return mResource; }
        ID3D12Resource* D3DResource() const { return mResource.Get(); }

        const D3D12_RESOURCE_DESC& GetDescription() const { return mDescription; }

        virtual Size GetSubresourcesCount() const = 0;

        virtual CopyCommandFunction GetCopyCommandFunction() const = 0;
    protected:
        ComPtr<ID3D12Resource> mResource;
        D3D12_RESOURCE_DESC mDescription;

    private:
        std::string mResourceName;
    };

} // namespace Engine::Memory