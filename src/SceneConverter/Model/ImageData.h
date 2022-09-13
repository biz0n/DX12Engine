#pragma once

#include <d3d12.h>
#include <vector>
#include <memory>
#include <string>

namespace SceneConverter::Model
{
    enum class TextureUsage
    {
        BaseColor = 1 << 0,
        Normal = 1 << 1,
        MetallicRoughness = 1 << 2,
        AmbientOcclusion = 1 << 3,
        Emissive = 1 << 4,
    };
    DEFINE_ENUM_FLAG_OPERATORS(TextureUsage);

    class ImageData
    {
    public:
        static std::shared_ptr<ImageData> LoadImageFromFile(const std::string& path);
        static std::shared_ptr<ImageData> LoadImageFromData(const std::vector<byte>& data, const std::string& extension, const std::string& name);

        ImageData() = default;
        ~ImageData() = default;

        void SetUseAs(TextureUsage usage) { mUsage |= usage; }
        bool IsUseAs(TextureUsage usage) const { return (mUsage & usage) == usage; }

        bool IsEmpty() const { return !mIsNotEmpty; }

        void SetFileName(const std::string& name) { mFileName = name; }
        const std::string& GetFileName() { return mFileName; }

        const std::string GetOriginalPath() const { return mPath; }
        const std::string GetExtension() const { return mExtension; }
        const std::vector<byte> GetData() const { return mData; }

    private:
        std::string mPath;
        std::string mExtension;
        std::vector<byte> mData = {};
        std::string mFileName;
        bool mIsNotEmpty;
        TextureUsage mUsage;
    };

} // namespace Engine::Scene