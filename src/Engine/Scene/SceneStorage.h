#pragma once

#include <Types.h>

#include <Scene/MeshResources.h>
#include <Scene/Material.h>
#include <Memory/Texture.h>

#include <vector>

namespace Engine::Scene
{
    struct SceneData;

    struct SceneData
    {
        SharedPtr<Memory::Texture> skyBoxTexture;
        SharedPtr<Memory::Texture> whiteTexture;
        SharedPtr<Memory::Texture> blackTexture;
        SharedPtr<Memory::Texture> fuchsiaTexture;
    };

    class SceneStorage
    {
    public:
        SceneStorage(std::vector<SharedPtr<Memory::Texture>>&& textures, std::vector<Material>&& materials, std::vector<MeshResources>&& meshes, SceneData&& sceneData);
        ~SceneStorage() = default;

    public:
        const std::vector<Scene::MeshResources>& GetMeshes() const { return mMeshes; }
        const std::vector<Material>& GetMaterials() const { return mMaterials; }
        const SceneData& GetSceneData() const { return mSceneData; }

    private:
        std::vector<SharedPtr<Memory::Texture>> mTextures;
        std::vector<Material> mMaterials;
        std::vector<MeshResources> mMeshes;
        SceneData mSceneData;
    }; 

    
}