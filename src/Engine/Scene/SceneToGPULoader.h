#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Bin3D/SceneStorage.h>
#include <Bin3D/PunctualLight.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Material.h>
#include <Scene/MeshResources.h>
#include <Scene/SceneStorage.h>

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
        static void CreateLightNode(Context& context, const Bin3D::PunctualLight& light, entt::entity entity);
        static void CreateMeshNode(Context& context, const Bin3D::Mesh& mesh, uint32_t meshIndex, entt::entity entity);
        static void CreateCameraNode(Context& context, const Bin3D::Camera& camera, entt::entity entity);

        SharedPtr<Memory::Texture> GetTexture(SharedPtr<Image> image);
        Material GetMaterial(Context& context, const Bin3D::Material& materialDto, const SceneData& sceneData);
        MeshResources GetMeshResources(Context& context, const Bin3D::Mesh& meshDto);
        SharedPtr<Memory::Texture> CreateTexture(DirectX::XMFLOAT4 color, String name);
        Engine::Scene::SceneData CreateSceneData(const SceneDataDto& sceneData);
    private:
        struct Context
        {
            entt::registry* registry;
            std::vector<SharedPtr<Memory::Texture>> textures;
            std::vector<Material> materials;
            std::vector<MeshResources> meshes;
            SharedPtr<Bin3D::SceneStorage> scene;
            bool isMainCameraAssigned;
        };

        Memory::ResourceFactory* mResourceFactory;
        Memory::ResourceCopyManager* mResourceCopyManager;
    };
}