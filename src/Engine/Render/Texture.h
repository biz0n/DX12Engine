#pragma once

#include <Types.h>

#include <Render/TextureCreationInfo.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DescriptorAllocator.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class Texture
    {
    public:
        Texture(ComPtr<ID3D12Device> device, std::string name, const TextureCreationInfo &creationInfo);
        ~Texture();

        D3D12_CPU_DESCRIPTOR_HANDLE GetRTDescriptor(DescriptorAllocator *allocator);
        D3D12_CPU_DESCRIPTOR_HANDLE GetDSDescriptor(DescriptorAllocator *allocator);
        D3D12_CPU_DESCRIPTOR_HANDLE GetSRDescriptor(DescriptorAllocator *allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc = nullptr);
        D3D12_CPU_DESCRIPTOR_HANDLE GetUADescriptor(DescriptorAllocator *allocator, const D3D12_UNORDERED_ACCESS_VIEW_DESC *desc = nullptr);

        ID3D12Resource* D3D12Resource() {return mResource.Get(); };

        ComPtr<ID3D12Resource> D3D12ResourceCom() {return mResource; };

    private:
        ComPtr<ID3D12Device> mDevice;
        ComPtr<ID3D12Resource> mResource;

        std::unordered_map<size_t, DescriptorAllocation> mSRDescriptors;
        std::unordered_map<size_t, DescriptorAllocation> mUADescriptors;
        DescriptorAllocation mRTDescriptor;
        DescriptorAllocation mDSDescriptor;
    };

} // namespace Engine::Render
