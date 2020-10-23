#pragma once

#include <Resource.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Scene/SceneForwards.h>

#include <vector>

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

        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(ComPtr<ID3D12Device> device, SharedPtr<DescriptorAllocator> allocator)
        {
            if (mAllocaion.GetNumDescsriptors() == 0)
            {
                auto allocation = allocator->Allocate();
                device->CreateShaderResourceView(GetD3D12Resource().Get(), nullptr, allocation.GetDescriptor());
                mAllocaion = std::move(allocation);
            }

            return mAllocaion.GetDescriptor();
        }

    private:
        DescriptorAllocation mAllocaion;
        bool isSRGB = false;
        SharedPtr<Scene::Image> mImage;
    };

} // namespace Engine::Scene