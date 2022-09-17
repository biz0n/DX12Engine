#include "SceneStorage.h"


#include <Scene/MeshResources.h>
#include <Memory/Texture.h>
#

#include <vector>

namespace Engine::Scene
{
    SceneStorage::SceneStorage(std::vector<SharedPtr<Memory::Texture>>&& textures, std::vector<Bin3D::Material>&& materials, std::vector<MeshResources>&& meshes, SceneData&& sceneData) :
        mTextures{ textures }, mMaterials{ materials }, mMeshes{ meshes }, mSceneData{ sceneData }
    {

    }
}