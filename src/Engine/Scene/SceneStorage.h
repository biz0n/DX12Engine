#pragma once

#include <Types.h>

#include <Scene/Mesh.h>
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
        SceneStorage(std::vector<SharedPtr<Memory::Texture>>&& textures, std::vector<Material>&& materials, std::vector<Mesh>&& meshes, SceneData&& sceneData);
        ~SceneStorage() = default;

    public:
        const std::vector<Scene::Mesh>& GetMeshes() const { return mMeshes; }
        const std::vector<Material>& GetMaterials() const { return mMaterials; }
        const SceneData& GetSceneData() const { return mSceneData; }

    private:
        std::vector<SharedPtr<Memory::Texture>> mTextures;
        std::vector<Material> mMaterials;
        std::vector<Mesh> mMeshes;
        SceneData mSceneData;
    }; 

    
}