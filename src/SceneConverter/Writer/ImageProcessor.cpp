#include "ImageProcessor.h"

#include <cstdio>
#include <filesystem>
#include <sstream>

#include <d3d11.h>
#include <DirectXTex.h>

#include <wrl.h>
#include <dxgi1_6.h>

namespace SceneConverter::Writer
{
    void ImageProcessor::ProcessImages(const std::filesystem::path& path, Model::Scene& scene)
    {
        Microsoft::WRL::ComPtr<ID3D11Device> device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
        D3D_FEATURE_LEVEL SupportedLevel;
        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            D3D11_CREATE_DEVICE_SINGLETHREADED,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &device,
            &SupportedLevel,
            &deviceContext);

        for (auto image : scene.GetImageResources())
        {
            if (image->IsEmpty() || image->IsUnused())
            {
                continue;
            }

            ProcessImage(device.Get(), path, image);
        }
        scene.FulfillImagePaths();
    }

    void ImageProcessor::ProcessImage(ID3D11Device* device, const std::filesystem::path& path, std::shared_ptr<Model::ImageData> image)
    {
        std::string extension = image->GetExtension();
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        DirectX::ScratchImage scratch;

        bool isHdr = false;
        if (image->GetData().empty())
        {
            std::filesystem::path imageOriginalPath = image->GetOriginalPath();
            if (extension == "dds")
            {
                DirectX::LoadFromDDSFile(imageOriginalPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, scratch);
            }
            else if (extension == "tga")
            {
                DirectX::LoadFromTGAFile(imageOriginalPath.c_str(), nullptr, scratch);
            }
            else if (extension == "hdr")
            {
                DirectX::LoadFromHDRFile(imageOriginalPath.c_str(), nullptr, scratch);
                bool isHdr = true;
            }
            else
            {
                DirectX::LoadFromWICFile(imageOriginalPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, scratch);
            }
        }
        else
        {
            if (extension == "dds")
            {
                DirectX::LoadFromDDSMemory(image->GetData().data(), image->GetData().size(), DirectX::DDS_FLAGS_NONE, nullptr, scratch);
            }
            else if (extension == "tga")
            {
                DirectX::LoadFromTGAMemory(image->GetData().data(), image->GetData().size(), nullptr, scratch);
            }
            else if (extension == "hdr")
            {
                DirectX::LoadFromHDRMemory(image->GetData().data(), image->GetData().size(), nullptr, scratch);
                bool isHdr = true;
            }
            else
            {
                DirectX::LoadFromWICMemory(image->GetData().data(), image->GetData().size(), DirectX::WIC_FLAGS_NONE, nullptr, scratch);
            }
        }

#if 0
        if (scratch.GetMetadata().mipLevels == 1)
        {
            DirectX::ScratchImage mipChain;
            GenerateMipMaps(
                scratch.GetImages(),
                scratch.GetImageCount(),
                scratch.GetMetadata(),
                DirectX::TEX_FILTER_DEFAULT,
                0,
                mipChain);

            scratch = std::move(mipChain);
        }

        if (!DirectX::IsCompressed(scratch.GetMetadata().format))
        {
            DirectX::ScratchImage compressed;
            DirectX::Compress(
                device,
                scratch.GetImages(),
                scratch.GetImageCount(),
                scratch.GetMetadata(),
                isHdr ? DXGI_FORMAT_BC6H_UF16 : DXGI_FORMAT_BC7_UNORM,
                DirectX::TEX_COMPRESS_DEFAULT,
                1.0,
                compressed
            );

            scratch = std::move(compressed);
        }
#endif
        const auto& imageMeta = scratch.GetMetadata();

        ImageMeta meta = {};
        meta.Width = imageMeta.width;
        meta.Height = imageMeta.height;
        meta.MipLevels = imageMeta.mipLevels;
        meta.IsCompressed = DirectX::IsCompressed(imageMeta.format);
        meta.IsHdr = isHdr;

        auto imagePath = GenerateImageName(path, image, meta);

        image->SetFileName(imagePath.filename().string());

        DirectX::SaveToDDSFile(scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(), DirectX::DDS_FLAGS_NONE, imagePath.c_str());
    }

    std::filesystem::path ImageProcessor::GenerateImageName(const std::filesystem::path& path, std::shared_ptr<const Model::ImageData> image, const ImageMeta& meta)
    {
        std::stringstream prefixBuilder;

        auto materialsCount = image->GetMaterialIds().size();
        for (size_t i = 0; i < materialsCount; ++i)
        {
            auto id = image->GetMaterialIds()[i];
            prefixBuilder << id;

            if (i < materialsCount - 1)
            {
                prefixBuilder << '_';
            }
        }

        if (image->IsUseAs(Model::TextureUsage::BaseColor))
        {
            prefixBuilder << "_basecolor";
        }
        if (image->IsUseAs(Model::TextureUsage::Normal))
        {
            prefixBuilder << "_normal";
        }
        if (image->IsUseAs(Model::TextureUsage::MetallicRoughness))
        {
            prefixBuilder << "_metallicroughness";
        }
        if (image->IsUseAs(Model::TextureUsage::AmbientOcclusion))
        {
            prefixBuilder << "_ao";
        }
        if (image->IsUseAs(Model::TextureUsage::Emissive))
        {
            prefixBuilder << "_emissive_";
        }

        prefixBuilder << '_' << meta.Width << 'x' << meta.Height;

        prefixBuilder << "_mips" << meta.MipLevels;

        if (meta.IsHdr)
        {
            prefixBuilder << "_hdr";
        }

        if (meta.IsCompressed)
        {
            prefixBuilder << "_compressed";
        }
        

        std::string prefix = prefixBuilder.str();

        std::string filename = prefix + ".dds";
        std::filesystem::path imagePath = path / filename;

        int index = 0;
        while (std::filesystem::exists(imagePath))
        {
            filename = std::format("{}.{}.dds", prefix, index);
            imagePath = path / filename;
            index++;
        }

        return imagePath;
    }
}
