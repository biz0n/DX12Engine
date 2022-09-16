#include "Image.h"

#include <filesystem>

namespace Engine::Scene
{
    SharedPtr<Image> Image::LoadImageFromFile(String path)
    {
        std::filesystem::path filename = path;

        auto extension = filename.extension();
        UniquePtr<DirectX::ScratchImage> scratch = MakeUnique<DirectX::ScratchImage>();

        HRESULT hr;
        if (extension == ".dds")
        {
            hr = DirectX::LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, *scratch);
        }
        else if (extension == ".hdr")
        {
            hr = DirectX::LoadFromHDRFile(filename.c_str(), nullptr, *scratch);
        }
        else
        {
            throw std::exception("Not supported format");
        }

        if (FAILED(hr))
        {
            throw std::bad_alloc();
        }

        SharedPtr<Image> image = MakeShared<Image>();
        image->mImage = std::move(scratch);
        image->SetName(path);

        return image;
    }

    SharedPtr<Image> Image::CreateFromColor(DirectX::XMFLOAT4 color, String name)
    {
        uint8_t maxValue = std::numeric_limits<uint8_t>::max();
        uint8_t c[4] = { maxValue * color.x, maxValue * color.y, maxValue * color.z, maxValue * color.w };

        UniquePtr<DirectX::ScratchImage> scratch = MakeUnique<DirectX::ScratchImage>();
        DirectX::Image dxImage = {};
        dxImage.width = 1;
        dxImage.height = 1;
        dxImage.rowPitch = 4;
        dxImage.format = DXGI_FORMAT_R8G8B8A8_UNORM;
        dxImage.pixels = &c[0];

        scratch->InitializeFromImage(dxImage, false);

        SharedPtr<Image> image = MakeShared<Image>();
        image->mImage = std::move(scratch);
        image->SetName(name);
        return image;
    }

    D3D12_RESOURCE_DESC Image::GetDescription(bool makeSRGB) const
    {
        const auto& metadata = mImage->GetMetadata();
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