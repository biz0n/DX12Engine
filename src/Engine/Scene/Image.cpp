#include "Image.h"

#include <filesystem>

namespace Engine::Scene
{
    SharedPtr<Image> Image::LoadImageFromFile(String path, bool generateMips)
    {
        std::filesystem::path filename = path;

        auto extension = filename.extension();
        UniquePtr<DirectX::ScratchImage> scratch = MakeUnique<DirectX::ScratchImage>();

        HRESULT hr;
        if (extension == ".dds")
        {
            hr = DirectX::LoadFromDDSFile(filename.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, *scratch);
        }
        else if (extension == ".tga")
        {
            hr = DirectX::LoadFromTGAFile(filename.c_str(), nullptr, *scratch);
        }
        else if (extension == ".hdr")
        {
            hr = DirectX::LoadFromHDRFile(filename.c_str(), nullptr, *scratch);
        }
        else
        {
            hr = DirectX::LoadFromWICFile(filename.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, *scratch);
        }

        if (FAILED(hr))
        {
            throw std::bad_alloc();
        }

        { /*
        UniquePtr<DirectX::ScratchImage> flippedImage = MakeUnique<DirectX::ScratchImage>();
        DirectX::FlipRotate(
            scratch->GetImages(),
            scratch->GetImageCount(),
            scratch->GetMetadata(),
            DirectX::TEX_FR_FLIP_VERTICAL,
            *flippedImage);

        scratch = std::move(flippedImage);
        */
        }

        if (generateMips && scratch->GetMetadata().mipLevels == 1)
        {
            UniquePtr<DirectX::ScratchImage> mipChain = MakeUnique<DirectX::ScratchImage>();
            GenerateMipMaps(
                scratch->GetImages(),
                scratch->GetImageCount(),
                scratch->GetMetadata(),
                DirectX::TEX_FILTER_DEFAULT,
                0,
                *mipChain);

            scratch = std::move(mipChain);
        }

       // DirectX::SaveToDDSFile(scratch->GetImages(),scratch->GetImageCount(), scratch->GetMetadata(), DirectX::DDS_FLAGS_NONE, dds.wstring().c_str());

        SharedPtr<Image> image = MakeShared<Image>();
        image->mImage = std::move(scratch);
        image->SetName(path);

        return image;
    }

    SharedPtr<Image> Image::LoadImageFromData(const std::vector<Byte> &imageData, String extension, String name, bool generateMips)
    {
        UniquePtr<DirectX::ScratchImage> scratch = MakeUnique<DirectX::ScratchImage>();
        HRESULT hr;
        if (extension == "dds")
        {
            hr = DirectX::LoadFromDDSMemory(imageData.data(), imageData.size(), DirectX::DDS_FLAGS_NONE, nullptr, *scratch);
        }
        else if (extension == "tga")
        {
            hr = DirectX::LoadFromTGAMemory(imageData.data(), imageData.size(), nullptr, *scratch);
        }
        else if (extension == "hdr")
        {
            hr = DirectX::LoadFromHDRMemory(imageData.data(), imageData.size(), nullptr, *scratch);
        }
        else
        {
            hr = DirectX::LoadFromWICMemory(imageData.data(), imageData.size(), DirectX::WIC_FLAGS_NONE, nullptr, *scratch);
        }

        if (FAILED(hr))
        {
            throw std::bad_alloc();
        }

        if (generateMips && scratch->GetMetadata().mipLevels == 1)
        {
            UniquePtr<DirectX::ScratchImage> mipChain = MakeUnique<DirectX::ScratchImage>();
            GenerateMipMaps(
                scratch->GetImages(),
                scratch->GetImageCount(),
                scratch->GetMetadata(),
                DirectX::TEX_FILTER_DEFAULT,
                0,
                *mipChain);

            scratch = std::move(mipChain);
        }

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