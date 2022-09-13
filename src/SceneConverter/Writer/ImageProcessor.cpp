#include "ImageProcessor.h"

#include <cstdio>
#include <filesystem>

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
            if (image->IsEmpty())
            {
                continue;
            }

            ProcessImage(device.Get(), path, image);
        }
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

        auto imagePath = GenerateImageName(path, image, scratch.GetMetadata().mipLevels, DirectX::IsCompressed(scratch.GetMetadata().format));

        image->SetFileName(imagePath.filename().string());

        DirectX::SaveToDDSFile(scratch.GetImages(), scratch.GetImageCount(), scratch.GetMetadata(), DirectX::DDS_FLAGS_NONE, imagePath.c_str());
	}

    std::filesystem::path ImageProcessor::GenerateImageName(const std::filesystem::path& path, std::shared_ptr<const Model::ImageData> image, int mipLevels, bool isCompressed)
    {
        std::string prefix = "";

        if (image->IsUseAs(Model::TextureUsage::BaseColor))
        {
            prefix += "basecolor_";
        }
        if (image->IsUseAs(Model::TextureUsage::Normal))
        {
            prefix += "normal_";
        }
        if (image->IsUseAs(Model::TextureUsage::MetallicRoughness))
        {
            prefix += "metallicroughness_";
        }
        if (image->IsUseAs(Model::TextureUsage::AmbientOcclusion))
        {
            prefix += "ao_";
        }
        if (image->IsUseAs(Model::TextureUsage::Emissive))
        {
            prefix += "emissive_";
        }

        prefix += std::format("mip{}_", mipLevels);

        if (isCompressed)
        {
            prefix += "compressed_";
        }

        std::filesystem::path imagePath = "";
        do
        {
            char tmpName[L_tmpnam];
            std::tmpnam(tmpName);
            std::filesystem::path file = tmpName;
            std::string filename = prefix + file.filename().string() + ".dds";
            imagePath = path / filename;
        } while (std::filesystem::exists(imagePath));

        return imagePath;
    }
}
