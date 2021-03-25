#pragma once

#include <Types.h>
#include <Name.h>

#include <Memory/MemoryForwards.h>
#include <Render/RenderForwards.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class FrameResourceProvider
    {
    public:
        FrameResourceProvider(ComPtr<ID3D12Device> device, Memory::ResourceFactory* resourceFactory);
        ~FrameResourceProvider();

        void CreateResource(const Name& name, const TextureCreationInfo& textureInfo, D3D12_RESOURCE_STATES state);

        Memory::Texture* GetTexture(const Name& name) const;
    private:
        ComPtr<ID3D12Device> mDevice;
        Memory::ResourceFactory* mResourceFactory;

        struct ResourceData;

        std::unordered_map<Name, ResourceData> mResources;

        struct ResourceData
        {
            size_t hash;
            SharedPtr<Memory::Texture> texture;
        };
    };
    
} // namespace Engine::Render
