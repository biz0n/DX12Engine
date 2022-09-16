#pragma once

#include <Types.h>

#include <d3d12.h>
#include <vector>
#include <DirectXTex.h>

namespace Engine::Scene
{
    class Image
    {
    public:
        static SharedPtr<Image> LoadImageFromFile(String path);
        static SharedPtr<Image> CreateFromColor(DirectX::XMFLOAT4 color, String name);

        Image() = default;
        ~Image() = default;

        const UniquePtr<DirectX::ScratchImage> &GetImage() const { return mImage; }

        void SetName(const String &name) { mName = name; }
        const String &GetName() const { return mName; }

        D3D12_RESOURCE_DESC GetDescription(bool makeSRGB) const;

    private:
        UniquePtr<DirectX::ScratchImage> mImage;
        String mName;
    };

} // namespace Engine::Scene