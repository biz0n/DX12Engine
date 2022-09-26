#pragma once

#include <Bin3D/Scene.h>

#include <filesystem>

namespace Bin3D::Reader
{
    class BinaryReader
    {
    public:
        std::shared_ptr<Scene> ReadScene(const std::filesystem::path& path);
    };
}

