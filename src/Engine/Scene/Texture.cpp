#include "Texture.h"

#include <Render/DirectXHashes.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine::Scene
{
    Texture::Texture(const std::wstring &name)
        : Resource(name)
    {
    }

    Texture::~Texture()
    {
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetShaderResourceView(ComPtr<ID3D12Device> device, SharedPtr<Memory::DescriptorAllocator> allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc)
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
                device->CreateShaderResourceView(mResource.Get(), desc, srDescriptor.GetDescriptor());

                iter = mSRDescriptors.insert({hash, std::move(srDescriptor)}).first;
            }

            return iter->second.GetDescriptor();
        }

} // namespace Engine::Scene