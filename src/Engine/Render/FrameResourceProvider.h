#pragma once

#include <Types.h>
#include <Name.h>

#include <Render/RenderForwards.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class FrameResourceProvider
    {
    public:
        FrameResourceProvider(ComPtr<ID3D12Device> device, GlobalResourceStateTracker* stateTracker);
        ~FrameResourceProvider();

        void CreateResource(const Name& name, const TextureCreationInfo& textureInfo);

        Texture* GetTexture(const Name& name) const;
    private:
        ComPtr<ID3D12Device> mDevice;
        GlobalResourceStateTracker* mStateTracker;

        struct ResourceData;

        std::unordered_map<Name, ResourceData> mResources;

        struct ResourceData
        {
            size_t hash;
            UniquePtr<Texture> texture;
        };
    };
    
} // namespace Engine::Render
