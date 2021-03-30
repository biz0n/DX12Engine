#include "FrameResourceProvider.h"

#include <HAL/DirectXHashes.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/Texture.h>
#include <Memory/ResourceFactory.h>
#include <Memory/TextureCreationInfo.h>

#include <tuple>

namespace Engine::Render
{
    FrameResourceProvider::FrameResourceProvider(ComPtr<ID3D12Device> device, Memory::ResourceFactory* resourceFactory)
        : mDevice{device}, mResourceFactory(resourceFactory)
    {
    }
    FrameResourceProvider::~FrameResourceProvider() = default;

    void FrameResourceProvider::CreateResource(const Name &name, const Memory::TextureCreationInfo &textureInfo, D3D12_RESOURCE_STATES state)
    {
        auto iter = mResources.find(name);
        size_t hash = std::hash<Memory::TextureCreationInfo>{}(textureInfo);

        if (iter != mResources.end())
        {
            if (iter->second.hash != hash)
            {
                iter->second.hash = hash;
                iter->second.texture = mResourceFactory->CreateTexture(textureInfo.description, state, &textureInfo.clearValue);
            }
        }
        else
        {
            FrameResourceProvider::ResourceData data = {};
            data.hash = hash;
            data.texture = mResourceFactory->CreateTexture(textureInfo.description, state, &textureInfo.clearValue);
            mResources.emplace(name, std::move(data));
        }
    }

    Memory::Texture* FrameResourceProvider::GetTexture(const Name &name) const
    {
        return  mResources.at(name).texture.get();
    }
} // namespace Engine::Render
