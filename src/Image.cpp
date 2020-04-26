#include "Image.h"

#include <filesystem>


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

    if (generateMips)
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
    image->SetImage(std::move(scratch));
    image->SetName(path);
    return image;
}

SharedPtr<Image> Image::LoadImageFromData(const std::vector<Byte>& imageData, String extension, String name, bool generateMips)
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

    if (generateMips)
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
    image->SetImage(std::move(scratch));
    image->SetName(name);
    return image;
}