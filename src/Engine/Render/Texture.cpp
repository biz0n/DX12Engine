#include "Texture.h"

#include <StringUtils.h>
#include <Exceptions.h>
#include <DirectXHashes.h>

#include <Memory/DescriptorAllocator.h>
#include <Render/TextureCreationInfo.h>

#include <d3dx12.h>

namespace Engine::Render
{
    Texture::Texture(ComPtr<ID3D12Device> device, std::string name, const TextureCreationInfo &creationInfo) : 
        mDevice{device}
    {
        CD3DX12_HEAP_PROPERTIES heapProperties{D3D12_HEAP_TYPE_DEFAULT};
        ThrowIfFailed(mDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &creationInfo.description,
            D3D12_RESOURCE_STATE_COMMON,
            &creationInfo.clearValue,
            IID_PPV_ARGS(&mResource)));
        
        mResource->SetName(StringToWString(name).c_str());
    }

    Texture::~Texture() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetRTDescriptor(Memory::DescriptorAllocator *allocator)
    {
        if (mRTDescriptor.IsNull())
        {
            mRTDescriptor = allocator->Allocate();
            mDevice->CreateRenderTargetView(mResource.Get(), nullptr, mRTDescriptor.GetDescriptor());
        }

        return mRTDescriptor.GetDescriptor();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDSDescriptor(Memory::DescriptorAllocator *allocator)
    {
        if (mDSDescriptor.IsNull())
        {
            mDSDescriptor = allocator->Allocate();
            mDevice->CreateDepthStencilView(mResource.Get(), nullptr, mDSDescriptor.GetDescriptor());
        }

        return mDSDescriptor.GetDescriptor();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetSRDescriptor(Memory::DescriptorAllocator *allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc)
    {
        size_t hash = 0;
        if (desc)
        {
            hash = std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>{}(*desc);
        }

        auto iter = mSRDescriptors.find(hash);
        if (iter == mSRDescriptors.end())
        {
            auto srDescriptor = allocator->Allocate();    
            mDevice->CreateShaderResourceView(mResource.Get(), desc, srDescriptor.GetDescriptor());

            iter = mSRDescriptors.insert({hash, std::move(srDescriptor)}).first;
        }

        return iter->second.GetDescriptor();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUADescriptor(Memory::DescriptorAllocator *allocator, const D3D12_UNORDERED_ACCESS_VIEW_DESC *desc)
    {
        size_t hash = 0;
        if (desc)
        {
            hash = std::hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>{}(*desc);
        }

        auto iter = mSRDescriptors.find(hash);
        if (iter == mSRDescriptors.end())
        {
            auto uaDescriptor = allocator->Allocate();    
            mDevice->CreateUnorderedAccessView(mResource.Get(), nullptr, desc, uaDescriptor.GetDescriptor());

            iter = mUADescriptors.insert({hash, std::move(uaDescriptor)}).first;
        }

        return iter->second.GetDescriptor();
    }
}