#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Bin3D/SceneStorage.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Material.h>
#include <Scene/Mesh.h>
#include <Scene/SceneStorage.h>
#include <Scene/PunctualLight.h>
#include <entt/entt.hpp>
#include <tuple>
#include <DirectXCollision.h>
#include <filesystem>

namespace Engine::Scene
{
    class SceneToGPULoader
    {
    public:
        struct SceneDataDto
        {
            String skyBoxPath;
        };
    private:
        struct Context;
    public:
        SceneToGPULoader(Memory::ResourceFactory* resourceFactory, Memory::ResourceCopyManager* resourceCopyManager);
        ~SceneToGPULoader() = default;
        SharedPtr<SceneStorage> LoadSceneToGPU(entt::registry& registry, SharedPtr<Bin3D::SceneStorage> scene, const SceneDataDto& sceneData);
    private:
        static void BuildNodeHierarchy(Context& context);
        static void CreateLightNode(Context& context, const Bin3D::PunctualLight& lightDto, entt::entity entity);
        static void CreateMeshNode(Context& context, const Bin3D::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateCameraNode(Context& context, const Bin3D::Camera& cameraDto, entt::entity entity);

        SharedPtr<Memory::Texture> GetTexture(SharedPtr<Image> image);
        Material GetMaterial(Context& context, const Bin3D::Material& materialDto, const SceneData& sceneData);
        Mesh GetMesh(Context& context, const Bin3D::Mesh& meshDto);
        SharedPtr<Memory::Texture> CreateTexture(DirectX::XMFLOAT4 color, String name);
        Engine::Scene::SceneData CreateSceneData(const SceneDataDto& sceneData);
    private:
        struct Context
        {
            entt::registry* registry;
            std::vector<SharedPtr<Memory::Texture>> textures;
            std::vector<Material> materials;
            std::vector<Mesh> meshes;
            std::vector<PunctualLight> lights;
            SharedPtr<Bin3D::SceneStorage> scene;
            bool isMainCameraAssigned;
        };

        Memory::ResourceFactory* mResourceFactory;
        Memory::ResourceCopyManager* mResourceCopyManager;
    };
}