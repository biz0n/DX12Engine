#pragma once

#include <Model/Scene.h>
#include <filesystem>

class ID3D11Device;

namespace SceneConverter::Writer
{
    class ImageProcessor
    {
    public:
        void ProcessImages(const std::filesystem::path& path, Model::Scene& scene);
    private:
        void ProcessImage(ID3D11Device* device, const std::filesystem::path& path, std::shared_ptr<Model::ImageData> image);

        struct ImageMeta;
        std::filesystem::path GenerateImageName(const std::filesystem::path& path, std::shared_ptr<const Model::ImageData> image, const ImageMeta& meta);

        struct ImageMeta
        {
            int Width;
            int Height;
            int MipLevels;
            bool IsCompressed;
            bool IsHdr;
        };
    };
}

