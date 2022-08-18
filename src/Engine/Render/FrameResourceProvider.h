#pragma once

#include <Types.h>
#include <Name.h>

#include <Memory/TextureCreationInfo.h>
#include <Memory/MemoryForwards.h>
#include <Render/RenderForwards.h>
#include <HAL/HALForwards.h>

#include <d3d12.h>
#include <unordered_map>
#include <functional>
#include <vector>
#include <queue>

namespace Engine::Render
{
    struct ScheduledResourceInfo
    {
        D3D12_RESOURCE_DIMENSION Dimension;
        UINT64 Width;
        UINT Height;
        UINT16 DepthOrArraySize;
        UINT16 MipLevels;
        DXGI_FORMAT Format;
        D3D12_RESOURCE_STATES InitialState;
        D3D12_RESOURCE_STATES ExpectedStates;
    };

    class FrameResourceProvider
    {
    private:
        struct ResourceData
        {
            size_t hash;
            ScheduledResourceInfo scheduledResourceInfo;
            std::vector<std::function<void(ScheduledResourceInfo&)>> propagators;
            Memory::TextureCreationInfo creationInfo;
            SharedPtr<Memory::Texture> texture;
            Name id;
        };
    public:
        FrameResourceProvider(ComPtr<ID3D12Device> device, SharedPtr<HAL::SwapChain> swapChain, Memory::ResourceFactory* resourceFactory);
        ~FrameResourceProvider();

        void QueueResourceCreate(const Name& name, const Memory::TextureCreationInfo& textureInfo, D3D12_RESOURCE_STATES state, std::function<void(ScheduledResourceInfo&)> propagate);
        void QueueResourceWrite(const Name& name, const Name& originalName, std::function<void(ScheduledResourceInfo&)> propagate);
        void QueueResourceRead(const Name& name, std::function<void(ScheduledResourceInfo&)> propagate);

        void PrepareResources();
        void CreateResources();

        Memory::Texture* GetTexture(const Name& name) const;
    private:
        ResourceData& GetResourceData(const Name& name, Optional<Name> originalName = std::nullopt);
        const Name GetOriginalName(const Name& name) const;
        const bool HasOriginalNameExists(const Name& name) const;
    private:
        ComPtr<ID3D12Device> mDevice;
        SharedPtr<HAL::SwapChain> mSwapChain;
        Memory::ResourceFactory* mResourceFactory;

        std::unordered_map<Name, Name> mAliasesMap;
        std::unordered_map<Name, Index> mResourcesMap;

        std::vector<ResourceData> mResources;

        std::queue<std::tuple<Name, Name, std::function<void(ScheduledResourceInfo&)>>> mWriteQueue;
        std::vector<std::tuple<Name, std::function<void(ScheduledResourceInfo&)>>> mReadQueue;
    };
    
} // namespace Engine::Render