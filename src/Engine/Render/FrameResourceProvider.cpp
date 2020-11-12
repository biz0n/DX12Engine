#include "FrameResourceProvider.h"

#include <tuple>

namespace Engine::Render
{
    FrameResourceProvider::FrameResourceProvider(ComPtr<ID3D12Device> device, GlobalResourceStateTracker* stateTracker)
        : mDevice{device}, mStateTracker(stateTracker)
    {
    }
    FrameResourceProvider::~FrameResourceProvider() = default;

    void FrameResourceProvider::CreateResource(const Name &name, const TextureCreationInfo &textureInfo)
    {
        auto iter = mResources.find(name);
        size_t hash = std::hash<TextureCreationInfo>{}(textureInfo);

        if (iter != mResources.end())
        {
            if (iter->second.hash != hash)
            {
                iter->second.hash = hash;
                iter->second.texture = MakeUnique<Texture>(mDevice, name.string(), textureInfo);
                mStateTracker->TrackResource(iter->second.texture->D3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
            }
        }
        else
        {
            FrameResourceProvider::ResourceData data = {};
            data.hash = hash;
            data.texture = MakeUnique<Texture>(mDevice, name.string(), textureInfo);
            mStateTracker->TrackResource(data.texture->D3D12Resource(), D3D12_RESOURCE_STATE_COMMON);
            mResources.emplace(name, std::move(data));
        }
    }

    Texture* FrameResourceProvider::GetTexture(const Name &name) const
    {
        return  mResources.at(name).texture.get();
    }
} // namespace Engine::Render
