#pragma once

#include <Types.h>

#include <Bin3D/Material.h>
#include <Scene/MeshResources.h>
#include <Memory/Texture.h>

#include <vector>

namespace Engine::Scene
{
    struct SceneData;

    struct SceneData
    {
        uint32_t skyBoxTextureIndex;
        uint32_t whiteTextureIndex;
        uint32_t blackTextureIndex;
        uint32_t fuchsiaTextureIndex;
    };

    class SceneStorage
    {
    public:
        SceneStorage(std::vector<SharedPtr<Memory::Texture>>&& textures, std::vector<Bin3D::Material>&& materials, std::vector<MeshResources>&& meshes, SceneData&& sceneData);
        ~SceneStorage() = default;

    public:
        const std::vector<Scene::MeshResources>& GetMeshes() const { return mMeshes; }
        const std::vector<Bin3D::Material>& GetMaterials() const { return mMaterials; }
        const SceneData& GetSceneData() const { return mSceneData; }
        bool HasTexture(uint32_t index) const { return index < mTextures.size() && mTextures[index] != nullptr; }
        SharedPtr<Memory::Texture> GetTexture(uint32_t index) const { return mTextures[index]; }

    private:
        std::vector<SharedPtr<Memory::Texture>> mTextures;
        std::vector<Bin3D::Material> mMaterials;
        std::vector<MeshResources> mMeshes;
        SceneData mSceneData;
    }; 

    
}