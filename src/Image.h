#pragma once

#include "Types.h"

#include <d3d12.h>
#include <vector>
#include <DirectXTex.h>

#define GENERATE_MIPS true


class Image
{
public:
    static SharedPtr<Image> LoadImageFromFile(String path, bool generateMips = GENERATE_MIPS);
    static SharedPtr<Image> LoadImageFromData(const std::vector<Byte>& data, String extension, String name, bool generateMips = GENERATE_MIPS);

    Image() = default;
    ~Image() = default;

    void SetImage(UniquePtr<DirectX::ScratchImage> image) { mImage = std::move(image); }
    const UniquePtr<DirectX::ScratchImage> &GetImage() const { return mImage; }

    void SetName(const String &name) { mName = name; }
    const String &GetName() const { return mName; }

private:
    UniquePtr<DirectX::ScratchImage> mImage;
    String mName;
};