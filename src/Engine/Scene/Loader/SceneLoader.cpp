#include "SceneLoader.h"

#include <Bin3D/Reader/BinaryReader.h>

namespace Engine::Scene::Loader
{
    SharedPtr<Bin3D::SceneStorage> SceneLoader::LoadScene(const String& path)
    {
        Bin3D::Reader::BinaryReader reader;

        return reader.ReadScene(path);
    }

} // namespace Engine::Scene::Loader