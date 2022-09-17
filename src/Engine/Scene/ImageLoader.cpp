#include "ImageLoader.h"

#include <filesystem>

namespace Engine::Scene
{
    SharedPtr<const DirectX::ScratchImage> ImageLoader::LoadImageFromFile(String path)
    {
        std::filesystem::path filename = path;

        auto extension = filename.extension();

        SharedPtr<DirectX::ScratchImage> image = MakeShared<DirectX::ScratchImage>();

        HRESULT hr;
        if (extension == ".dds")
        {
            hr = DirectX::LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, *image);
        }
        else if (extension == ".hdr")
        {
            hr = DirectX::LoadFromHDRFile(filename.c_str(), nullptr, *image);
        }
        else
        {
            throw std::exception("Not supported format");
        }

        if (FAILED(hr))
        {
            throw std::bad_alloc();
        }

        return image;
    }

    SharedPtr<const DirectX::ScratchImage> ImageLoader::CreateFromColor(DirectX::XMFLOAT4 color)
    {
        uint8_t maxValue = std::numeric_limits<uint8_t>::max();
        uint8_t c[4] = { maxValue * color.x, maxValue * color.y, maxValue * color.z, maxValue * color.w };

        SharedPtr<DirectX::ScratchImage> image = MakeShared<DirectX::ScratchImage>();
        DirectX::Image dxImage = {};
        dxImage.width = 1;
        dxImage.height = 1;
        dxImage.rowPitch = 4;
        dxImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        dxImage.pixels = &c[0];

        image->InitializeFromImage(dxImage, false);

        return image;
    }

    D3D12_RESOURCE_DESC ImageLoader::GetDescription(SharedPtr<const DirectX::ScratchImage> image, bool makeSRGB)
    {
        const auto& metadata = image->GetMetadata();
        DXGI_FORMAT format = metadata.format;
        if (makeSRGB)
        {
            format = DirectX::MakeSRGB(format);
        }

        D3D12_RESOURCE_DESC desc = {};
        desc.Width = static_cast<uint32>(metadata.width);
        desc.Height = static_cast<uint32>(metadata.height);
        desc.MipLevels = static_cast<uint16>(metadata.mipLevels);
        desc.DepthOrArraySize = (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
            ? static_cast<uint16>(metadata.depth)
            : static_cast<uint16>(metadata.arraySize);
        desc.Format = format;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.SampleDesc.Count = 1;
        desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

        return desc;
    }

} // namespace Engine::Scene