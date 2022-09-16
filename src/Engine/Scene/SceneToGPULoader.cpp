#include "SceneToGPULoader.h"

#include <StringUtils.h>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/AABBComponent.h>
#include <Scene/SceneStorage.h>

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
    SceneToGPULoader::SceneToGPULoader(Memory::ResourceFactory* resourceFactory, Memory::ResourceCopyManager* resourceCopyManager) :
    mResourceFactory{resourceFactory}, mResourceCopyManager{resourceCopyManager}
    {

    }

    SharedPtr<SceneStorage> SceneToGPULoader::LoadSceneToGPU(entt::registry& registry, SharedPtr<Bin3D::SceneStorage> scene, const SceneDataDto& sceneDataDto)
    {
        Context context = {};
        context.registry = &registry;
        context.scene = scene;
        context.isMainCameraAssigned = false;

        SceneData sceneData = CreateSceneData(sceneDataDto);

        context.textures.reserve(scene->GetImagePaths().size());
        for (const auto& imageName : scene->GetImagePaths())
        {
            std::string name = context.scene->GetString(imageName.PathIndex).data();
            if (name != "")
            {
                std::filesystem::path imagePath = context.scene->GetPath() / name;
                auto image = Image::LoadImageFromFile(imagePath.string());
                context.textures.push_back(GetTexture(image));
            }
            else
            {
                context.textures.push_back(nullptr);
            }
        }

        context.materials.reserve(scene->GetMaterials().size());
        for (const auto& material : scene->GetMaterials())
        {
            context.materials.push_back(GetMaterial(context, material, sceneData));
        }

        context.meshes.reserve(scene->GetMeshes().size());
        for (const auto& mesh : scene->GetMeshes())
        {
            context.meshes.push_back(GetMeshResources(context, mesh));
        }

        BuildNodeHierarchy(context);

        SharedPtr<SceneStorage> sceneStorage = MakeShared<SceneStorage>(
            std::move(context.textures), 
            std::move(context.materials), 
            std::move(context.meshes), 
            std::move(sceneData));
        return sceneStorage;

    }

    void SceneToGPULoader::BuildNodeHierarchy(Context& context)
    {
        auto scene = context.scene;
        auto registry = context.registry;
        
        auto nodes = scene->GetNodes();
        auto count = nodes.size();

        std::vector<entt::entity> entities;
        std::vector<Engine::Scene::Components::RelationshipComponent> relationships;
        std::vector<size_t> lastChild;

        entities.reserve(count);
        relationships.resize(count, {});
        lastChild.resize(count, 0);

        for (size_t i = 0; i < count; ++i)
        {
            entities.push_back(context.registry->create());
            const auto& node = nodes[i];
            auto entity = entities[i];

            std::string nodeName = context.scene->GetString(node.NameIndex).data();
            registry->emplace<Components::NameComponent>(entity, nodeName);

            DirectX::XMMATRIX localTransform = DirectX::XMLoadFloat4x4(&node.LocalTransform);
            
            registry->emplace<Scene::Components::LocalTransformComponent>(entity, localTransform);

            if (node.Parent == i)
            {
                context.registry->emplace<Components::Root>(entities[i]);
            }
            else
            {
                auto& nodeRelationship = relationships[i];
                auto& parentRelationship = relationships[node.Parent];

                nodeRelationship.parent = entities[node.Parent];
                nodeRelationship.depth = parentRelationship.depth + 1;
                parentRelationship.childsCount++;

                if (parentRelationship.first == entt::null)
                {
                    parentRelationship.first = entity;
                }
                else
                {
                    auto& lastChildRelationship = relationships[lastChild[node.Parent]];
                    lastChildRelationship.next = entity;
                }
                lastChild[node.Parent] = i;
            }

            if (node.Type == Bin3D::Node::NodeType::Mesh)
            {
                CreateMeshNode(context, node, entity, &relationships[i]);
            }
            else if (node.Type == Bin3D::Node::NodeType::Light)
            {
                auto& light = scene->GetLights()[node.DataIndex.Offset];
                CreateLightNode(context, light, entity);
            }
            else if (node.Type == Bin3D::Node::NodeType::Camera)
            {
                auto& camera = scene->GetCameras()[node.DataIndex.Offset];
                CreateCameraNode(context, camera, entity);
            }
        }

        for (size_t i = 0; i < count; ++i)
        {
            registry->emplace<Components::RelationshipComponent>(entities[i], relationships[i]);
        }

    }

    void SceneToGPULoader::CreateLightNode(Context& context, const Bin3D::PunctualLight& light, entt::entity entity)
    {
        Components::LightComponent lightComponent;
        lightComponent.light = light;

        context.registry->emplace<Components::LightComponent>(entity, lightComponent);
        context.registry->emplace<Components::CameraComponent>(entity, Components::CameraComponent{});
    }

    void SceneToGPULoader::CreateMeshNode(Context& context, const Bin3D::Node& node, entt::entity entity, Engine::Scene::Components::RelationshipComponent* relationship)
    {
        auto nodeMeshIndices = context.scene->GetMeshIndices(node.DataIndex);

        auto numMeshes = nodeMeshIndices.size();
        entt::entity nextEntity = context.registry->create();
        relationship->first = nextEntity;
        relationship->childsCount = numMeshes;

        for (uint32 i = 0; i < numMeshes; ++i)
        {
            auto meshIndex = nodeMeshIndices[i];
            const auto& meshDto = context.scene->GetMeshes()[meshIndex];
                
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

            context.registry->emplace<Components::NameComponent>(meshEntity, std::string(context.scene->GetString(meshDto.NameIndex)));

            Components::RelationshipComponent meshRelationship;
            meshRelationship.next = nextEntity;
            meshRelationship.parent = entity;
            meshRelationship.depth = relationship->depth + 1;
            context.registry->emplace<Components::RelationshipComponent>(meshEntity, meshRelationship);

            Components::MeshComponent meshComponent;
            meshComponent.MeshIndex = meshIndex;
            meshComponent.MaterialIndex = meshDto.MaterialIndex;
            meshComponent.VerticesCount = meshDto.Vertices.Size;
            meshComponent.IndicesCount = meshDto.Indices.Size;

            context.registry->emplace<Components::MeshComponent>(meshEntity, meshComponent);

            Components::AABBComponent aabbComponent = {};
            aabbComponent.originalBoundingBox = meshDto.AABB;
            aabbComponent.boundingBox = meshDto.AABB;
            context.registry->emplace<Components::AABBComponent>(meshEntity, aabbComponent);
        }
    }

    void SceneToGPULoader::CreateCameraNode(Context& context, const Bin3D::Camera& camera, entt::entity entity)
    {
        Components::CameraComponent cameraComponent;
        cameraComponent.camera = camera;

        context.registry->emplace<Components::CameraComponent>(entity, cameraComponent);

        if (!context.isMainCameraAssigned)
        {
            context.isMainCameraAssigned = true;
            context.registry->emplace<Components::MainCameraComponent>(entity);
        }
    }

    SharedPtr<Memory::Texture> SceneToGPULoader::GetTexture(SharedPtr<Image> image)
    {
        auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        auto texture = mResourceFactory->CreateTexture(image->GetDescription(false), state);
        texture->SetName(image->GetName());

        ComPtr<ID3D12Device> device;
        texture->D3DResource()->GetDevice(IID_PPV_ARGS(&device));

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

    SharedPtr<Memory::Texture> SceneToGPULoader::CreateTexture(DirectX::XMFLOAT4 color, String name)
    {
        return GetTexture(Image::CreateFromColor(color, name));
    }

    Material SceneToGPULoader::GetMaterial(Context& context, const Bin3D::Material& materialDto, const SceneData& sceneData)
    {
        Material material = {};
        material.SetProperties(materialDto.MaterialProperties);

        if (materialDto.BaseColorTextureIndex)
        {
            auto texture = context.textures[materialDto.BaseColorTextureIndex];
            material.SetBaseColorTexture(texture);
        }

        if (materialDto.NormalTextureIndex)
        {
            auto texture = context.textures[materialDto.NormalTextureIndex];
            material.SetNormalTexture(texture);
        }

        if (materialDto.MetallicRoughnessTextureIndex)
        {
            auto texture = context.textures[materialDto.MetallicRoughnessTextureIndex];
            material.SetMetallicRoughnessTexture(texture);
        }

        if (materialDto.AmbientOcclusionTextureIndex)
        {
            auto texture = context.textures[materialDto.AmbientOcclusionTextureIndex];
            material.SetAmbientOcclusionTexture(texture);
        }

        if (materialDto.EmissiveTextureIndex)
        {
            auto texture = context.textures[materialDto.EmissiveTextureIndex];
            material.SetEmissiveTexture(texture);
        }

        return material;
    }

    MeshResources SceneToGPULoader::GetMeshResources(Context& context, const Bin3D::Mesh& meshDto)
    {
        auto indices = context.scene->GetIndices(meshDto.Indices);
        auto vertices = context.scene->GetVertices(meshDto.Vertices);

        using TIndexType = typename std::decay<decltype(*indices.begin())>::type;
        using TVertexType = typename std::decay<decltype(*vertices.begin())>::type;

        std::string meshName = context.scene->GetString(meshDto.NameIndex).data();

        MeshResources mesh;
        mesh.indexBuffer = mResourceFactory->CreateIndexBuffer(indices.size(), sizeof(TIndexType), D3D12_RESOURCE_STATE_COMMON);
        mesh.indexBuffer->SetName("Indices: " + meshName);

        mesh.vertexBuffer = mResourceFactory->CreateVertexBuffer(vertices.size(), sizeof(TVertexType), D3D12_RESOURCE_STATE_COMMON);
        mesh.vertexBuffer->SetName("Vertices: " + meshName);
        
        Memory::Buffer::ScheduleUploading(
            mResourceFactory, 
            mResourceCopyManager, 
            mesh.indexBuffer.get(), 
            indices.data(), 
            indices.size_bytes(),
            D3D12_RESOURCE_STATE_INDEX_BUFFER);

        Memory::Buffer::ScheduleUploading(
            mResourceFactory, 
            mResourceCopyManager, 
            mesh.vertexBuffer.get(), 
            vertices.data(), 
            vertices.size_bytes(),
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        return mesh;
    }

    Engine::Scene::SceneData SceneToGPULoader::CreateSceneData(const SceneDataDto& sceneDataDto)
    {
        SceneData sceneData = {};

        sceneData.whiteTexture = CreateTexture({ 1.f, 1.f, 1.f, 1.f }, "White");
        sceneData.blackTexture = CreateTexture({ 0.f, 0.f, 0.f, 1.f }, "Black");
        sceneData.fuchsiaTexture = CreateTexture({ 1.f, 0.f, 1.f, 1.f }, "Fuchsia");

        if (std::filesystem::exists(sceneDataDto.skyBoxPath))
        {
            auto image = Scene::Image::LoadImageFromFile(sceneDataDto.skyBoxPath);
            sceneData.skyBoxTexture = GetTexture(image);
            sceneData.skyBoxTexture->SetName(image->GetName());
        }

        return sceneData;
    }
}
