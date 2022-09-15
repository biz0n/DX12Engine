#pragma once

#include <Types.h>

namespace Bin3D
{
    class SceneStorage;
}

namespace Engine::Scene::Loader
{
    class SceneLoader
    {
    public:
        SharedPtr<Bin3D::SceneStorage> LoadScene(const String& path);
    }; 
} // namespace Engine::Scene