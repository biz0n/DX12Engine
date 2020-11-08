#pragma once

#include <Types.h>

#include <Scene/SceneForwards.h>

#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>
#include <tuple>
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
enum aiTextureType;

namespace Engine::Scene::Loader
{
    class SceneLoader
    {
    private:
        struct LoadingContext
        {
            String RootPath;
            std::vector<std::tuple<String, Mesh, dx::BoundingBox>> meshes;
            std::vector<SharedPtr<Material>> materials;
            std::vector<SharedPtr<Texture>> dataTextures;
            std::unordered_map<String, SharedPtr<Scene::Texture>> fileTextures;
            std::unordered_map<String, aiLight*> lightsMap;
            std::unordered_map<String, aiCamera*> camerasMap;

            entt::registry* registry;
        };

            //! Values for the Sampler::magFilter field
    enum class SamplerMagFilter : unsigned int
    {
        UNSET = 0,
        SamplerMagFilter_Nearest = 9728,
        SamplerMagFilter_Linear = 9729
    };

    //! Values for the Sampler::minFilter field
    enum class SamplerMinFilter : unsigned int
    {
        UNSET = 0,
        SamplerMinFilter_Nearest = 9728,
        SamplerMinFilter_Linear = 9729,
        SamplerMinFilter_Nearest_Mipmap_Nearest = 9984,
        SamplerMinFilter_Linear_Mipmap_Nearest = 9985,
        SamplerMinFilter_Nearest_Mipmap_Linear = 9986,
        SamplerMinFilter_Linear_Mipmap_Linear = 9987
    };

    public:
        UniquePtr<SceneObject> LoadScene(String path, Optional<float32> scale = {});

    private:
        void ParseNode(const aiScene* aScene, const aiNode* aNode, const LoadingContext& context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        SharedPtr<Texture> GetTexture(const aiString& path, LoadingContext& context);
        SharedPtr<Texture> GetTexture(const aiTexture* aTexture, const LoadingContext& context);
        SharedPtr<Material> ParseMaterial(const aiMaterial* aMaterial, LoadingContext& context);
        void ParseSampler(const aiMaterial* aMaterial, aiTextureType textureType, unsigned int idx);
        std::tuple<String, Mesh, dx::BoundingBox> ParseMesh(const aiMesh* aMesh, const LoadingContext& context);
        bool IsLightNode(const aiNode* aNode, const LoadingContext& context);
        bool IsMeshNode(const aiNode* aNode, const LoadingContext& context);
        bool IsCameraNode(const aiNode* aNode, const LoadingContext& context);
        void CreateLightNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity);
        void CreateMeshNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        void CreateCameraNode(const aiNode* aNode, const LoadingContext& context, entt::entity entity);
    }; 
} // namespace Engine::Scene