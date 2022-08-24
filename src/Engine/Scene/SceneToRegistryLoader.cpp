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
#include <Scene/SceneStorage.h>

#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>
#include <Scene/Image.h>

#include <Memory/Texture.h>
#include <Memory/Buffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/IndexBuffer.h>
#include <Memory/ResourceFactory.h>
#include <Memory/ResourceCopyManager.h>
#include <Memory/UploadBuffer.h>

#include <entt/entt.hpp>
#include <vector>
#include <filesystem>
#include <type_traits>

namespace Engine::Scene
{
    SceneToRegisterLoader::SceneToRegisterLoader(Memory::ResourceFactory* resourceFactory, Memory::ResourceCopyManager* resourceCopyManager) :
    mResourceFactory{resourceFactory}, mResourceCopyManager{resourceCopyManager}
    {

    }

    SceneToRegisterLoader::~SceneToRegisterLoader() = default;

    SharedPtr<SceneStorage> SceneToRegisterLoader::LoadSceneToRegistry(entt::registry &registry, const Loader::SceneDto &scene)
    {
        Context context = {};
        context.registry = &registry;
        context.scene = &scene;
        context.isMainCameraAssigned = false;

        context.textures.reserve(scene.ImageResources.size());
        for (const auto& image : scene.ImageResources)
        {
            context.textures.push_back(GetTexture(image));
        }

        context.materials.reserve(scene.Materials.size());
        for (const auto& material : scene.Materials)
        {
            context.materials.push_back(GetMaterial(context, material));
        }

        context.meshes.reserve(scene.Meshes.size());
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

        SharedPtr<SceneStorage> sceneStorage = MakeShared<SceneStorage>();
        sceneStorage->mMaterials = std::move(context.materials);
        sceneStorage->mMeshes = std::move(context.meshes);
        sceneStorage->mTextures = std::move(context.textures);

        return std::move(sceneStorage);

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
            const auto& meshDto = context.scene->Meshes[meshIndex];
                
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

            context.registry->emplace<Components::NameComponent>(meshEntity, meshDto.Name);

            Components::RelationshipComponent meshRelationship;
            meshRelationship.next = nextEntity;
            meshRelationship.parent = entity;
            meshRelationship.depth = relationship->depth + 1;
            context.registry->emplace<Components::RelationshipComponent>(meshEntity, meshRelationship);

            Components::MeshComponent meshComponent;
            meshComponent.MeshIndex = meshIndex;
            meshComponent.MaterialIndex = meshDto.MaterialIndex;
            meshComponent.VerticesCount = meshDto.Vertices.size();
            meshComponent.IndicesCount = meshDto.Indices.size();

            context.registry->emplace<Components::MeshComponent>(meshEntity, meshComponent);

            Components::AABBComponent aabbComponent = {};
            aabbComponent.originalBoundingBox = meshDto.AABB;
            aabbComponent.boundingBox = meshDto.AABB;
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

    SharedPtr<Memory::Texture> SceneToRegisterLoader::GetTexture(const Loader::ImageDto& imageDto)
    {
        auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        auto texture = mResourceFactory->CreateTexture(imageDto.Image->GetDescription(false), state);
        texture->SetName(imageDto.Image->GetName());

        ComPtr<ID3D12Device> device;
        texture->D3DResource()->GetDevice(IID_PPV_ARGS(&device));

        const auto image = imageDto.Image;
        const auto &metadata = image->GetImage()->GetMetadata();
        const DirectX::Image *images = image->GetImage()->GetImages();
        const Size imageCount = image->GetImage()->GetImageCount();

        std::vector<D3D12_SUBRESOURCE_DATA> subresources;
        ThrowIfFailed(PrepareUpload(
                device.Get(),
                images,
                imageCount,
                metadata,
                subresources));

        const auto numSubresources = static_cast<unsigned int>(subresources.size());

        uint64 requiredSize = 0;
        std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
        std::vector<uint32> numRows;
        std::vector<uint64> rowSizesInBytes;

        layouts.resize(numSubresources);
        numRows.resize(numSubresources);
        rowSizesInBytes.resize(numSubresources);

        auto resourceDesc = texture->GetDescription();
        device->GetCopyableFootprints(
                &resourceDesc,
                0,
                numSubresources,
                0,
                layouts.data(),
                numRows.data(),
                rowSizesInBytes.data(),
                &requiredSize);

        auto uploadBuffer = mResourceFactory->CreateUploadBuffer(requiredSize);
        auto allocation = uploadBuffer->Allocate(requiredSize, 1);

        for (UINT i = 0; i < numSubresources; ++i)
        {
            D3D12_MEMCPY_DEST DestData =
            {
                allocation.CPU + layouts[i].Offset,
                layouts[i].Footprint.RowPitch,
                static_cast<size_t>(layouts[i].Footprint.RowPitch) * static_cast<size_t>(numRows[i])
            };
            MemcpySubresource(&DestData, &subresources[i], static_cast<size_t>(rowSizesInBytes[i]), numRows[i], layouts[i].Footprint.Depth);
        }

        Memory::ResourceCopyData copyData{};
        copyData.DestinationResource = texture->D3DResource();
        copyData.UploadBuffer = uploadBuffer;
        copyData.StateAfterCopy = state;
        copyData.CopyCommand = texture->GetCopyCommandFunction();

        mResourceCopyManager->ScheduleWriting(copyData);

        return texture;
    }

    Material SceneToRegisterLoader::GetMaterial(Context& context, const Loader::MaterialDto& materialDto)
    {
        Material material = {};
        material.SetProperties(materialDto.MaterialProperties);

        if (materialDto.BaseColorTextureIndex)
        {
            auto texture = context.textures[materialDto.BaseColorTextureIndex.value()];
            //texture->SetSRGB(true);
            material.SetBaseColorTexture(texture);
        }

        if (materialDto.NormalTextureIndex)
        {
            auto texture = context.textures[materialDto.NormalTextureIndex.value()];
            material.SetNormalTexture(texture);
        }

        if (materialDto.MetallicRoughnessTextureIndex)
        {
            auto texture = context.textures[materialDto.MetallicRoughnessTextureIndex.value()];
            material.SetMetallicRoughnessTexture(texture);
        }

        if (materialDto.AmbientOcclusionTextureIndex)
        {
            auto texture = context.textures[materialDto.AmbientOcclusionTextureIndex.value()];
            material.SetAmbientOcclusionTexture(texture);
        }

        if (materialDto.EmissiveTextureIndex)
        {
            auto texture = context.textures[materialDto.EmissiveTextureIndex.value()];
            material.SetEmissiveTexture(texture);
        }

        return material;
    }

    Mesh SceneToRegisterLoader::GetMesh(Context& context, const Loader::MeshDto& meshDto)
    {
        using TIndexType = typename std::decay<decltype(*meshDto.Indices.begin())>::type;
        using TVertexType = typename std::decay<decltype(*meshDto.Vertices.begin())>::type;
        const Size indicesCount = meshDto.Indices.size();
        const Size verticesCount = meshDto.Vertices.size();

        Mesh mesh;
        mesh.indexBuffer = mResourceFactory->CreateIndexBuffer(indicesCount, sizeof(TIndexType), D3D12_RESOURCE_STATE_COMMON);
        mesh.indexBuffer->SetName("Indices: " + meshDto.Name);

        mesh.vertexBuffer = mResourceFactory->CreateVertexBuffer(verticesCount, sizeof(TVertexType), D3D12_RESOURCE_STATE_COMMON);
        mesh.vertexBuffer->SetName("Vertices: " + meshDto.Name);

        mesh.verticesCount = verticesCount;
        mesh.indicesCount = indicesCount;

        Memory::Buffer::ScheduleUploading(mResourceFactory, mResourceCopyManager, mesh.indexBuffer.get(), meshDto.Indices.data(), indicesCount * sizeof(TIndexType), D3D12_RESOURCE_STATE_INDEX_BUFFER);
        Memory::Buffer::ScheduleUploading(mResourceFactory, mResourceCopyManager, mesh.vertexBuffer.get(), meshDto.Vertices.data(), verticesCount * sizeof(TVertexType), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        return mesh;
    }

    void SceneToRegisterLoader::AddCubeMapToScene(entt::registry &registry, String texturePath)
    {
        if (!std::filesystem::exists(texturePath))
        {
            return;
        }

        CubeMap cubeMap;
        auto image = Scene::Image::LoadImageFromFile(texturePath);
        Loader::ImageDto imageDto {image};
        cubeMap.texture = GetTexture(imageDto);
        cubeMap.texture->SetName(image->GetName());

        auto cubeMapEntity = registry.create();

        registry.emplace<Components::CubeMapComponent>(cubeMapEntity, cubeMap);
    }
}
