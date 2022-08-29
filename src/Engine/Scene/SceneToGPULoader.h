#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Scene/Loader/SceneDto.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Material.h>
#include <Scene/Mesh.h>
#include <Scene/SceneStorage.h>
#include <Scene/PunctualLight.h>
#include <entt/fwd.hpp>
#include <tuple>
#include <DirectXCollision.h>

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
        SharedPtr<SceneStorage> LoadSceneToGPU(entt::registry& registry, const Loader::SceneDto& scene, const SceneDataDto& sceneData);
    private:
        bool ParseNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateLightNode(Context& context, const Loader::LightDto& lightDto, entt::entity entity);
        static void CreateMeshNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateCameraNode(Context& context, const Loader::CameraDto& cameraDto, entt::entity entity);

        SharedPtr<Memory::Texture> GetTexture(const Loader::ImageDto& imageDto);
        Material GetMaterial(Context& context, const Loader::MaterialDto& materialDto, const SceneData& sceneData);
        Mesh GetMesh(Context& context, const Loader::MeshDto& meshDto);
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
            const Loader::SceneDto* scene;
            bool isMainCameraAssigned;
        };

        Memory::ResourceFactory* mResourceFactory;
        Memory::ResourceCopyManager* mResourceCopyManager;
    };
}