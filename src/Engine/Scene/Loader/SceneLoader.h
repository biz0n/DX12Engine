#pragma once

#include <Types.h>

#include <Scene/SceneForwards.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <vector>
#include <unordered_map>

#include <entt/fwd.hpp>
#include <Scene/Components/RelationshipComponent.h>

struct aiScene;
struct aiNode;
struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiString;
struct aiLight;
struct aiCamera;

namespace Engine::Scene::Loader
{
    class SceneLoader
    {
    private:
        struct LoadingContext
        {
            String RootPath;
            std::vector<SharedPtr<Mesh>> meshes;
            std::vector<SharedPtr<Material>> materials;
            std::vector<SharedPtr<Texture>> textures;
            std::unordered_map<String, SharedPtr<Scene::Image>> images;
            std::unordered_map<String, aiLight*> lightsMap;
            std::unordered_map<String, aiCamera*> camerasMap;

            entt::registry* registry;
        };

    public:
        UniquePtr<SceneObject> LoadScene(String path, Optional<float32> scale = {});

    private:
        void ParseNode(const aiScene* aScene, const aiNode* aNode, SceneObject* scene, SharedPtr<Node> parentNode, const LoadingContext& context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        SharedPtr<Texture> GetTexture(const aiString& path, LoadingContext& context);
        SharedPtr<Texture> GetTexture(const aiTexture* aTexture, const LoadingContext& context);
        SharedPtr<Material> ParseMaterial(const aiMaterial* aMaterial, LoadingContext& context);
        SharedPtr<Mesh> ParseMesh(const aiMesh* aMesh, const LoadingContext& context);
        bool IsLightNode(const aiNode* aNode, const LoadingContext& context);
        bool IsMeshNode(const aiNode* aNode, const LoadingContext& context);
        bool IsCameraNode(const aiNode* aNode, const LoadingContext& context);
        SharedPtr<LightNode> CreateLightNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity);
        SharedPtr<MeshNode> CreateMeshNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        SharedPtr<CameraNode> CreateCameraNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity);
    }; 
} // namespace Engine::Scene