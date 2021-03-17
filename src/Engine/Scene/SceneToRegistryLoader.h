#pragma once

#include <Types.h>

#include <Scene/Loader/SceneDto.h>
#include <Scene/Components/ComponentsForwards.h>
#include <Scene/Texture.h>
#include <Scene/Material.h>
#include <Scene/Mesh.h>
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
        void LoadSceneToRegistry(entt::registry& registry, const Loader::SceneDto& scene);
        void AddCubeMapToScene(entt::registry& registry, String texturePath);
    private:
        bool ParseNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateLightNode(Context& context, const Loader::LightDto& lightDto, entt::entity entity);
        static void CreateMeshNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship);
        static void CreateCameraNode(Context& context, const Loader::CameraDto& cameraDto, entt::entity entity);

        SharedPtr<Texture> GetTexture(const Loader::ImageDto& imageDto);
        SharedPtr<Material> GetMaterial(Context& context, const Loader::MaterialDto& materialDto);
        std::tuple<String, Mesh, dx::BoundingBox> GetMesh(Context& context, const Loader::MeshDto& meshDto);
    private:
        struct Context
        {
            entt::registry* registry;
            std::vector<SharedPtr<Texture>> textures;
            std::vector<SharedPtr<Material>> materials;
            std::vector<std::tuple<String, Mesh, dx::BoundingBox>> meshes;
            const Loader::SceneDto* scene;
            bool isMainCameraAssigned;
        };
    };
}