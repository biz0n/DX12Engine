#pragma once

#include <Types.h>

#include <DirectXTex.h>
#include <d3d12.h>
#include <filesystem>

namespace Engine::Scene
{
    class ImageLoader
    {
    public:
        static SharedPtr<const DirectX::ScratchImage> LoadImageFromFile(const std::filesystem::path& path);
        static SharedPtr<const DirectX::ScratchImage> CreateFromColor(DirectX::XMFLOAT4 color);

        static D3D12_RESOURCE_DESC GetDescription(SharedPtr<const DirectX::ScratchImage> image, bool makeSRGB);
    private:
        ImageLoader() = delete;
    };

} // namespace Engine::Scene