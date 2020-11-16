#pragma once

#include <Memory/Resource.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Scene/SceneForwards.h>
#include <Render/DirectXHashes.h>

#include <vector>
#include <unordered_map>

namespace Engine::Scene
{
    class Texture : public Resource
    {
    public:
        Texture(const std::wstring &name);
        virtual ~Texture();

        void SetImage(SharedPtr<Scene::Image> buffer)
        {
            mImage = buffer;
        }

        const SharedPtr<Scene::Image> &GetImage() const { return mImage; }

        inline bool IsSRGB() const { return isSRGB; }
        inline void SetSRGB(bool srgb) { isSRGB = srgb; }

        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(ComPtr<ID3D12Device> device, SharedPtr<DescriptorAllocator> allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc = nullptr)
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

    private:
        DescriptorAllocation mAllocaion;
        std::unordered_map<size_t, DescriptorAllocation> mSRDescriptors;
        bool isSRGB = false;
        SharedPtr<Scene::Image> mImage;
    };

} // namespace Engine::Scene