#include "FrameResourceProvider.h"

#include <HAL/DirectXHashes.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/Texture.h>
#include <Memory/ResourceFactory.h>
#include <Memory/TextureCreationInfo.h>

#include <HAL/SwapChain.h>

#include <tuple>
#include <queue>

namespace Engine::Render
{
    FrameResourceProvider::FrameResourceProvider(ComPtr<ID3D12Device> device, SharedPtr<HAL::SwapChain> swapChain, Memory::ResourceFactory* resourceFactory)
        : mDevice{ device }, mSwapChain{swapChain}, mResourceFactory(resourceFactory)
    {
    }
    FrameResourceProvider::~FrameResourceProvider() = default;

    void FrameResourceProvider::QueueResourceCreate(const Name& name, const Memory::TextureCreationInfo& textureInfo, D3D12_RESOURCE_STATES state, std::function<void(ScheduledResourceInfo&)> propagate)
    {
        auto& resourceData = GetResourceData(name);
        resourceData.creationInfo = textureInfo;
        resourceData.scheduledResourceInfo.InitialState = state;
        resourceData.propagators.push_back(propagate);
        resourceData.id = name;
    }

    void FrameResourceProvider::QueueResourceWrite(const Name& name, const Name& originalName, std::function<void(ScheduledResourceInfo&)> propagate)
    {
        mWriteQueue.push({ name, originalName, std::move(propagate) });
    }

    void FrameResourceProvider::QueueResourceRead(const Name& name, std::function<void(ScheduledResourceInfo&)> propagate)
    {
        mReadQueue.push_back({ name, std::move(propagate) });
    }

    void FrameResourceProvider::PrepareResources()
    {
        for (auto& creationInfo : mResources)
        {
            creationInfo.propagators.clear();
            creationInfo.scheduledResourceInfo = {};
            creationInfo.creationInfo = {};
            creationInfo.id = {};
        }

        mReadQueue.clear();
        
        while (!mWriteQueue.empty())
        {
            mWriteQueue.pop();
        }
    }

    void FrameResourceProvider::CreateResources()
    {
        for (auto& creationInfo : mResources)
        {
            for (auto& propagateFunc : creationInfo.propagators)
            {
                propagateFunc(creationInfo.scheduledResourceInfo);
            }
        }

        while (!mWriteQueue.empty())
        {
            auto& front = mWriteQueue.front(); 
            
            auto& originalName = std::get<1>(front);
            auto& name = std::get<0>(front);
            auto func = std::move(std::get<2>(front));
            auto originalNameStr = originalName.string();
            if (HasOriginalNameExists(originalName))
            {
                auto& resourceData = GetResourceData(std::get<0>(front), originalName);
                func(resourceData.scheduledResourceInfo);
            }
            else
            {
                mWriteQueue.push({name, originalName, func});
            }
            mWriteQueue.pop();
        }

        for (auto& read : mReadQueue)
        {
            auto restr = std::get<0>(read).string();
            auto& resourceData = GetResourceData(std::get<0>(read));
            auto func = std::move(std::get<1>(read));
            func(resourceData.scheduledResourceInfo);
        }

        for (auto& creationInfo : mResources)
        {
            if (creationInfo.creationInfo.description.Width == 0 || creationInfo.creationInfo.description.Height == 0)
            {
                creationInfo.creationInfo.description.Width = mSwapChain->GetWidth();
                creationInfo.creationInfo.description.Height = mSwapChain->GetHeight();
            }
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;


            if ((creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_DEPTH_WRITE) == D3D12_RESOURCE_STATE_DEPTH_WRITE ||
                (creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_DEPTH_READ) == D3D12_RESOURCE_STATE_DEPTH_READ)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

                if ((creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE &&
                    (creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) != D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)
                {
                    flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
                }
            }

            if ((creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_RENDER_TARGET) == D3D12_RESOURCE_STATE_RENDER_TARGET)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            }

            if ((creationInfo.scheduledResourceInfo.ExpectedStates & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            creationInfo.creationInfo.description.Flags = flags;

            size_t hash = std::hash<Memory::TextureCreationInfo>{}(creationInfo.creationInfo);
            auto* clearValue = creationInfo.creationInfo.clearValue.has_value() ? &creationInfo.creationInfo.clearValue.value() : nullptr;
            
            if (creationInfo.hash != hash)
            {
                D3D12_RESOURCE_DESC description = creationInfo.creationInfo.description;
                creationInfo.texture = mResourceFactory->CreateTexture(description, creationInfo.scheduledResourceInfo.InitialState, clearValue);
                creationInfo.texture->SetName(creationInfo.id.string());
                creationInfo.hash = hash;
            }
        }
    }

    FrameResourceProvider::ResourceData& FrameResourceProvider::GetResourceData(const Name& name, std::optional<Name> originalName)
    {
        auto resourceName = name;
        auto resourceIndexIt = mResourcesMap.find(resourceName);
        if (resourceIndexIt == mResourcesMap.end())
        {
            if (originalName.has_value())
            {
                mAliasesMap.insert({ name, *originalName });
                resourceName = GetOriginalName(*originalName);
            }
            else
            {
                resourceName = GetOriginalName(name);
            }
            resourceIndexIt = mResourcesMap.find(resourceName);
        }

        Index resourceIndex = 0;
        if (resourceIndexIt == mResourcesMap.end())
        {
            resourceIndex = mResources.size();
            mResourcesMap.insert({ resourceName, resourceIndex });
            mResources.push_back({});
        }
        else
        {
            resourceIndex = resourceIndexIt->second;
        }

        return mResources[resourceIndex];
    }

    const Name FrameResourceProvider::GetOriginalName(const Name& name) const
    {
        auto originalName = name;

        auto result = mAliasesMap.find(name);
        while (result != mAliasesMap.end())
        {
            originalName = result->second;
            result = mAliasesMap.find(originalName);
        }

        return originalName;
    }

    const bool FrameResourceProvider::HasOriginalNameExists(const Name& name) const
    {
        Name originalName = GetOriginalName(name);

        return mResourcesMap.find(originalName) != mResourcesMap.end();
    }

    Memory::Texture* FrameResourceProvider::GetTexture(const Name& name) const
    {
        const Name& originalName = GetOriginalName(name);
        Index index = mResourcesMap.at(originalName);
        return mResources[index].texture.get();
    }
} // namespace Engine::Render
