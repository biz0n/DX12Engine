#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Scene/Loader/SceneDto.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Material.h>
#include <Scene/Mesh.h>
#include <Scene/SceneStorage.h>
#include <entt/fwd.hpp>
#include <tuple>
#include <DirectXCollision.h>

namespace Engine::Scene
{
    class SceneToRegisterLoader
    {
    private:
        struct Context;
    public:
        SceneToRegisterLoader(Memory::ResourceFactory* resourceFactory, Memory::ResourceCopyManager* resourceCopyManager);
        ~SceneToRegisterLoader();
        SharedPtr<SceneStorage> LoadSceneToRegistry(entt::registry& registry, const Loader::SceneDto& scene);
        void AddCubeMapToScene(entt::registry& registry, String texturePath);
    private:
        bool ParseNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateLightNode(Context& context, const Loader::LightDto& lightDto, entt::entity entity);
        static void CreateMeshNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateCameraNode(Context& context, const Loader::CameraDto& cameraDto, entt::entity entity);

        SharedPtr<Memory::Texture> GetTexture(const Loader::ImageDto& imageDto);
        Material GetMaterial(Context& context, const Loader::MaterialDto& materialDto);
        Mesh GetMesh(Context& context, const Loader::MeshDto& meshDto);
    private:
        struct Context
        {
            entt::registry* registry;
            std::vector<SharedPtr<Memory::Texture>> textures;
            std::vector<Material> materials;
            std::vector<Mesh> meshes;
            const Loader::SceneDto* scene;
            bool isMainCameraAssigned;
        };

        Memory::ResourceFactory* mResourceFactory;
        Memory::ResourceCopyManager* mResourceCopyManager;
    };
}