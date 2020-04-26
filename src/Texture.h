#pragma once

#include "Resource.h"
#include "DescriptorAllocator.h"


#include <vector>

class Image;

class Texture : public Resource
{
public:
    Texture(const std::wstring& name);
    virtual ~Texture();

    void SetImage(SharedPtr<Image> buffer)
    {
        mImage = buffer;
    }

    const SharedPtr<Image>& GetImage() const {return mImage;}

    inline bool IsSRGB() const { return isSRGB; }
    inline void SetSRGB(bool srgb) { isSRGB = srgb; }

    DescriptorAllocation allocaion;

private:
    bool isSRGB = false;
    SharedPtr<Image> mImage;
};
