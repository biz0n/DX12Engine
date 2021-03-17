#include "SceneToRegistryLoader.h"

#include <StringUtils.h>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/CubeMapComponent.h>

#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>
#include <Scene/Image.h>

#include <Memory/VertexBuffer.h>
#include <Memory/IndexBuffer.h>

#include <entt/entt.hpp>
#include <vector>
#include <filesystem>

namespace Engine::Scene
{
    void SceneToRegisterLoader::LoadSceneToRegistry(entt::registry &registry, const Loader::SceneDto &scene)
    {
        Context context = {};
        context.registry = &registry;
        context.scene = &scene;
        context.isMainCameraAssigned = false;

        for (const auto& image : scene.ImageResources)
        {
            context.textures.push_back(GetTexture(image));
        }

        for (const auto& material : scene.Materials)
        {
            context.materials.push_back(GetMaterial(context, material));
        }

        for (const auto& mesh : scene.Meshes)
        {
            context.meshes.push_back(GetMesh(context, mesh));
        }

        for (const auto& node : scene.RootNodes)
        {
            auto rootEntity = registry.create();
            Engine::Scene::Components::RelationshipComponent rootRelationship;
            ParseNode(context, node, rootEntity, &rootRelationship);

            context.registry->emplace<Components::RelationshipComponent>(rootEntity, rootRelationship);
            context.registry->emplace<Components::Root>(rootEntity);
        }
    }

    bool SceneToRegisterLoader::ParseNode(Context& context, const Loader::Node &node, entt::entity entity, Engine::Scene::Components::RelationshipComponent *relationship)
    {
        auto registry = context.registry;
        const auto scene = context.scene;
        registry->emplace<Components::NameComponent>(entity, node.Name);
        registry->emplace<Scene::Components::LocalTransformComponent>(entity, node.LocalTransform);

        if (!node.MeshIndices.empty())
        {
            CreateMeshNode(context, node, entity, relationship);
        }
        else if (node.LightIndex)
        {
            auto& light = scene->Lights[node.LightIndex.value()];
            CreateLightNode(context, light, entity);
        }
        else if (node.CameraIndex)
        {
            auto& camera = scene->Cameras[node.CameraIndex.value()];
            CreateCameraNode(context, camera, entity);
        }
        else if (!node.Children.empty())
        {
            auto numChildren = node.Children.size();
            entt::entity nextEntity = numChildren > 0 ? context.registry->create() : entt::null;
            relationship->first = nextEntity;
            relationship->childsCount = numChildren;

            for (auto i = 0; i < numChildren; i++)
            {
                auto child = node.Children[i];

                auto childEntity = nextEntity;

                if (i < (numChildren - 1))
                {
                    nextEntity = context.registry->create();
                }
                else
                {
                    nextEntity = entt::null;
                }

                Components::RelationshipComponent childRelationship;
                childRelationship.parent = entity;
                childRelationship.next = nextEntity;
                childRelationship.depth = relationship->depth + 1;

                ParseNode(context, child, childEntity, &childRelationship);

                context.registry->emplace<Components::RelationshipComponent>(childEntity, childRelationship);
            }
        }
        else
        {
            return false;
        }

        return true;
    }

    void SceneToRegisterLoader::CreateLightNode(Context& context, const Loader::LightDto& lightDto, entt::entity entity)
    {
        PunctualLight light;

        light.SetEnabled(true);

        light.SetLightType(lightDto.LightType);
        light.SetDirection(lightDto.Direction);
        light.SetColor(lightDto.Color);
        light.SetIntensity(lightDto.Intensity);
        light.SetConstantAttenuation(lightDto.ConstantAttenuation);
        light.SetLinearAttenuation(lightDto.LinearAttenuation);
        light.SetQuadraticAttenuation(lightDto.QuadraticAttenuation);
        light.SetInnerConeAngle(lightDto.InnerConeAngle);
        light.SetOuterConeAngle(lightDto.OuterConeAngle);

        Components::LightComponent lightComponent;
        lightComponent.light = light;

        context.registry->emplace<Components::LightComponent>(entity, lightComponent);
        context.registry->emplace<Components::CameraComponent>(entity, Camera());
    }

    void SceneToRegisterLoader::CreateMeshNode(Context& context, const Loader::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship)
    {
        auto numMeshes = node.MeshIndices.size();
        entt::entity nextEntity = context.registry->create();;
        relationship->first = nextEntity;
        relationship->childsCount = numMeshes;

        for (uint32 i = 0; i < numMeshes; ++i)
        {
            auto meshIndex = node.MeshIndices[i];
            auto meshData = context.meshes[meshIndex];

            auto meshEntity = nextEntity;
            if (i < (numMeshes - 1))
            {
                nextEntity = context.registry->create();
            }
            else
            {
                nextEntity = entt::null;
            }

            context.registry->emplace<Components::LocalTransformComponent>(meshEntity, DirectX::XMMatrixIdentity());

            context.registry->emplace<Components::NameComponent>(meshEntity, std::get<0>(meshData));

            Components::RelationshipComponent meshRelationship;
            meshRelationship.next = nextEntity;
            meshRelationship.parent = entity;
            meshRelationship.depth = relationship->depth + 1;
            context.registry->emplace<Components::RelationshipComponent>(meshEntity, meshRelationship);

            Components::MeshComponent meshComponent;
            meshComponent.mesh = std::get<1>(meshData);

            context.registry->emplace<Components::MeshComponent>(meshEntity, meshComponent);

            Components::AABBComponent aabbComponent = {};
            aabbComponent.originalBoundingBox = std::get<2>(meshData);
            aabbComponent.boundingBox = std::get<2>(meshData);
            context.registry->emplace<Components::AABBComponent>(meshEntity, aabbComponent);
        }
    }

    void SceneToRegisterLoader::CreateCameraNode(Context& context, const Loader::CameraDto& cameraDto, entt::entity entity)
    {
        Camera camera;

        camera.SetNearPlane(cameraDto.NearPlane);
        camera.SetFarPlane(cameraDto.FarPlane);
        camera.SetFoV(cameraDto.FoV);
        camera.SetType(cameraDto.Type);

        Components::CameraComponent cameraComponent;
        cameraComponent.camera = camera;

        context.registry->emplace<Components::CameraComponent>(entity, cameraComponent);

        if (!context.isMainCameraAssigned)
        {
            context.isMainCameraAssigned = true;
            context.registry->emplace<Components::MainCameraComponent>(entity);
        }
    }

    SharedPtr<Texture> SceneToRegisterLoader::GetTexture(const Loader::ImageDto& imageDto)
    {
        SharedPtr<Texture> texture = MakeShared<Texture>(StringToWString(imageDto.Image->GetName()));
        texture->SetImage(imageDto.Image);

        return texture;
    }

    SharedPtr<Material> SceneToRegisterLoader::GetMaterial(Context& context, const Loader::MaterialDto& materialDto)
    {
        SharedPtr<Material> material = MakeShared<Material>();
        material->SetProperties(materialDto.MaterialProperties);

        if (materialDto.BaseColorTextureIndex)
        {
            auto texture = context.textures[materialDto.BaseColorTextureIndex.value()];
            texture->SetSRGB(true);
            material->SetBaseColorTexture(texture);
        }

        if (materialDto.NormalTextureIndex)
        {
            auto texture = context.textures[materialDto.NormalTextureIndex.value()];
            material->SetNormalTexture(texture);
        }

        if (materialDto.MetallicRoughnessTextureIndex)
        {
            auto texture = context.textures[materialDto.MetallicRoughnessTextureIndex.value()];
            material->SetMetallicRoughnessTexture(texture);
        }

        if (materialDto.AmbientOcclusionTextureIndex)
        {
            auto texture = context.textures[materialDto.AmbientOcclusionTextureIndex.value()];
            material->SetAmbientOcclusionTexture(texture);
        }

        if (materialDto.EmissiveTextureIndex)
        {
            auto texture = context.textures[materialDto.EmissiveTextureIndex.value()];
            material->SetEmissiveTexture(texture);
        }

        return material;
    }

    std::tuple<String, Mesh, dx::BoundingBox> SceneToRegisterLoader::GetMesh(Context& context, const Loader::MeshDto& meshDto)
    {
        Mesh mesh;
        mesh.indexBuffer = MakeShared<Memory::IndexBuffer>();
        mesh.vertexBuffer = MakeShared<Memory::VertexBuffer>();

        mesh.primitiveTopology = meshDto.PrimitiveTopology;
        mesh.material = context.materials[meshDto.MaterialIndex];

        mesh.vertexBuffer->SetData(meshDto.Vertices);
        mesh.vertexBuffer->SetName(StringToWString("Vertices: " + meshDto.Name));

        mesh.indexBuffer->SetData(meshDto.Indices);
        mesh.indexBuffer->SetName(StringToWString("Indices: " + meshDto.Name));

        return std::make_tuple(meshDto.Name, mesh, meshDto.AABB);
    }

    void SceneToRegisterLoader::AddCubeMapToScene(entt::registry &registry, String texturePath)
    {
        if (!std::filesystem::exists(texturePath))
        {
            return;
        }

        CubeMap cubeMap;
        auto image = Scene::Image::LoadImageFromFile(texturePath);
        SharedPtr<Texture> texture = MakeShared<Texture>(StringToWString(image->GetName()));
        texture->SetImage(image);
        texture->SetSRGB(true);

        cubeMap.texture = texture;
        cubeMap.primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        cubeMap.indexBuffer = MakeShared<Memory::IndexBuffer>(L"CubeMap Indices");
        cubeMap.vertexBuffer = MakeShared<Memory::VertexBuffer>(L"CubeMap Vertices");

        std::vector<Vertex> vertices = {};
        vertices.reserve(8);
        vertices.assign(8, {});

        auto* v = vertices.data();

        float w = 1.0f;
        float h = 1.0f;
        float d = 1.0f;

        // Fill in the front face vertex data.
        v[0].Vertex = {-w, -h, -d};
        v[1].Vertex = {-w, +h, -d};
        v[2].Vertex = {+w, +h, -d};
        v[3].Vertex = {+w, -h, -d};

        // Fill in the back face vertex data.
        v[4].Vertex = {-w, -h, +d};
        v[5].Vertex = {+w, -h, +d};
        v[6].Vertex = {+w, +h, +d};
        v[7].Vertex = {-w, +h, +d};

        std::vector<uint16> indices = {};
        indices.reserve(36);
        indices.assign(36, 0);

        auto* i = indices.data();

        // Fill in the front face index data
        i[0] = 0; i[1] = 1; i[2] = 2;
        i[3] = 0; i[4] = 2; i[5] = 3;

        // Fill in the back face index data
        i[6] = 4; i[7]  = 5; i[8]  = 6;
        i[9] = 4; i[10] = 6; i[11] = 7;

        // Fill in the top face index data
        i[12] = 1; i[13] =  7; i[14] = 6;
        i[15] = 1; i[16] = 6; i[17] = 2;

        // Fill in the bottom face index data
        i[18] = 0;  i[19] = 3; i[20] = 5;
        i[21] = 0;  i[22] = 5; i[23] = 4;

        // Fill in the left face index data
        i[24] = 4; i[25] = 7; i[26] = 1;
        i[27] = 4; i[28] = 1;  i[29] = 0;

        // Fill in the right face index data
        i[30] = 3;  i[31] = 2;  i[32] = 6;
        i[33] = 3;  i[34] = 6; i[35] = 5;

        cubeMap.vertexBuffer->SetData(vertices);

        cubeMap.indexBuffer->SetData(indices);

        auto cubeMapEntity = registry.create();

        registry.emplace<Components::CubeMapComponent>(cubeMapEntity, cubeMap);
    }
}
