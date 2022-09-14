#pragma once

#include <Scene/SceneStorage.h>

#include <filesystem>

namespace Engine::Scene::Reader
{
    class BinaryReader
    {
    public:
        SceneStorage ReadScene(const std::filesystem::path& path);
    };
}

