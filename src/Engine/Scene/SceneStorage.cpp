#include "SceneStorage.h"


#include <Scene/Mesh.h>
#include <Scene/Material.h>
#include <Memory/Texture.h>
#

#include <vector>

namespace Engine::Scene
{
    SceneStorage::SceneStorage(std::vector<SharedPtr<Memory::Texture>>&& textures, std::vector<Material>&& materials, std::vector<Mesh>&& meshes, SceneData&& sceneData) :
        mTextures{ textures }, mMaterials{ materials }, mMeshes{ meshes }, mSceneData{ sceneData }
    {

    }
}