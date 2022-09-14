#pragma once

#include <Bin3D/SceneStorage.h>

#include <filesystem>

namespace Bin3D::Reader
{
    class BinaryReader
    {
    public:
        SceneStorage ReadScene(const std::filesystem::path& path);
    };
}

