#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Bin3D/SceneStorage.h>
#include <Bin3D/PunctualLight.h>
#include <Bin3D/Material.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/MeshResources.h>
#include <Scene/SceneStorage.h>

#include <entt/entt.hpp>
#include <tuple>
#include <DirectXCollision.h>
#include <DirectXTex.h>
#include <filesystem>

namespace Engine::Scene
{
    class SceneToGPULoader
    {
    public:
        struct SceneDataDto
        {
            std::string skyBoxPath;
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

        uint32_t CreateTexture(Context& context, SharedPtr<const DirectX::ScratchImage> image, const std::string& name);
        MeshResources GetMeshResources(Context& context, const Bin3D::Mesh& meshDto);
        Engine::Scene::SceneData CreateSceneData(Context& context, const SceneDataDto& sceneData);
        SharedPtr<Memory::Buffer> CreateBuffer(Size bufferSize, Size strideSize, const void* data);
    private:
        struct Context
        {
            entt::registry* registry;
            std::vector<SharedPtr<Memory::Texture>> textures;
            std::vector<Bin3D::Material> materials;
            std::vector<MeshResources> meshes;
            SharedPtr<Bin3D::SceneStorage> scene;
            bool isMainCameraAssigned;
        };

        Memory::ResourceFactory* mResourceFactory;
        Memory::ResourceCopyManager* mResourceCopyManager;
    };
}