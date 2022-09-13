#include "ImageData.h"

#include <filesystem>

namespace SceneConverter::Model
{
    std::shared_ptr<ImageData> ImageData::LoadImageFromFile(const std::string& path)
    {
        std::filesystem::path filename = path;
        auto extension = filename.extension();

        auto image = std::make_shared<ImageData>();
        image->mPath = path;
        image->mExtension = extension.string().substr(1);
        image->mIsNotEmpty = true;

        return image;
    }

    std::shared_ptr<ImageData> ImageData::LoadImageFromData(const std::vector<byte>& imageData, const std::string& extension, const std::string& name)
    {
        auto image = std::make_shared<ImageData>();
        image->mPath = name;
        image->mExtension = extension;
        image->mData = imageData;
        image->mIsNotEmpty = true;

        return image;
    }
} // namespace Engine::Scene