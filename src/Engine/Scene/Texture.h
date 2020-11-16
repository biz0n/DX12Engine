#pragma once

#include <Memory/Resource.h>
#include <Memory/MemoryForwards.h>
#include <Memory/DescriptorAllocation.h>
#include <Scene/SceneForwards.h>


#include <vector>
#include <unordered_map>

namespace Engine::Scene
{
    class Texture : public Memory::Resource
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

        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(ComPtr<ID3D12Device> device, SharedPtr<Memory::DescriptorAllocator> allocator, const D3D12_SHADER_RESOURCE_VIEW_DESC *desc = nullptr);

    private:
        Memory::DescriptorAllocation mAllocaion;
        std::unordered_map<size_t, Memory::DescriptorAllocation> mSRDescriptors;
        bool isSRGB = false;
        SharedPtr<Scene::Image> mImage;
    };

} // namespace Engine::Scene