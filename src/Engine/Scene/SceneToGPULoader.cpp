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

#include <Scene/ImageLoader.h>

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

        context.textures.reserve(scene->GetImagePaths().size());
        for (const auto& imageName : scene->GetImagePaths())
        {
            std::string name = context.scene->GetString(imageName.PathIndex).data();
            if (name != "")
            {
                std::filesystem::path imagePath = context.scene->GetPath() / name;
                std::string imagePathStr = imagePath.string();
                auto image = ImageLoader::LoadImageFromFile(imagePathStr);
                CreateTexture(context, image, imagePathStr);
               
            }
            else
            {
                CreateTexture(context, nullptr, "");
            }
        }

        SceneData sceneData = CreateSceneData(context, sceneDataDto);

        context.materials.reserve(scene->GetMaterials().size());
        for (const auto& material : scene->GetMaterials())
        {
            context.materials.push_back(material);
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
                const auto& mesh = scene->GetMeshes()[node.DataIndex];
                CreateMeshNode(context, mesh, node.DataIndex, entity);

                context.meshes[node.DataIndex].indexBuffer->SetName("Indices: " + nodeName);
                context.meshes[node.DataIndex].vertexCoordinatesBuffer->SetName("Vertices coords: " + nodeName);
                context.meshes[node.DataIndex].vertexPropertiesBuffer->SetName("Vertices properties: " + nodeName);
            }
            else if (node.Type == Bin3D::Node::NodeType::Light)
            {
                const auto& light = scene->GetLights()[node.DataIndex];
                CreateLightNode(context, light, entity);
            }
            else if (node.Type == Bin3D::Node::NodeType::Camera)
            {
                const auto& camera = scene->GetCameras()[node.DataIndex];
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

    void SceneToGPULoader::CreateMeshNode(Context& context, const Bin3D::Mesh& mesh, uint32_t meshIndex, entt::entity entity)
    {
        Components::MeshComponent meshComponent;
        meshComponent.MeshIndex = meshIndex;
        meshComponent.MaterialIndex = mesh.MaterialIndex;

        context.registry->emplace<Components::MeshComponent>(entity, meshComponent);

        Components::AABBComponent aabbComponent = {};
        aabbComponent.originalBoundingBox = mesh.AABB;
        aabbComponent.boundingBox = mesh.AABB;
        context.registry->emplace<Components::AABBComponent>(entity, aabbComponent);
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

    uint32_t SceneToGPULoader::CreateTexture(Context& context, SharedPtr<const DirectX::ScratchImage> image, const std::string& name)
    {
        if (image == nullptr)
        {
            context.textures.push_back(nullptr);
            return context.textures.size() - 1;
        }

        auto state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
        auto texture = mResourceFactory->CreateTexture(ImageLoader::GetDescription(image, false), state);
        texture->SetName(name);

        ComPtr<ID3D12Device> device;
        texture->D3DResource()->GetDevice(IID_PPV_ARGS(&device));

        const auto &metadata = image->GetMetadata();
        const DirectX::Image *images = image->GetImages();
        const Size imageCount = image->GetImageCount();

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

        context.textures.push_back(texture);

        return context.textures.size() - 1;
    }

    MeshResources SceneToGPULoader::GetMeshResources(Context& context, const Bin3D::Mesh& meshDto)
    {
        auto indices = context.scene->GetIndices(meshDto.Indices);
        auto verticesCoordinates = context.scene->GetVerticesCoordinates(meshDto.Vertices);
        auto verticesProperties = context.scene->GetVerticesProperties(meshDto.Vertices);

        using TIndexType = typename std::decay<decltype(*indices.begin())>::type;
        using TVertexCoordinatesType = typename std::decay<decltype(*verticesCoordinates.begin())>::type;
        using TVertexPropertiesType = typename std::decay<decltype(*verticesProperties.begin())>::type;

        MeshResources mesh;
        mesh.indexBuffer = mResourceFactory->CreateBuffer(
            sizeof(TIndexType), 
            CD3DX12_RESOURCE_DESC::Buffer(indices.size_bytes()), 
            D3D12_RESOURCE_STATE_COMMON);

        mesh.vertexCoordinatesBuffer = mResourceFactory->CreateBuffer(
            sizeof(TVertexCoordinatesType), 
            CD3DX12_RESOURCE_DESC::Buffer(verticesCoordinates.size_bytes()),
            D3D12_RESOURCE_STATE_COMMON);

        mesh.vertexPropertiesBuffer = mResourceFactory->CreateBuffer(
            sizeof(TVertexPropertiesType),
            CD3DX12_RESOURCE_DESC::Buffer(verticesProperties.size_bytes()),
            D3D12_RESOURCE_STATE_COMMON);
        
        Memory::Buffer::ScheduleUploading(
            mResourceFactory, 
            mResourceCopyManager, 
            mesh.indexBuffer.get(), 
            indices.data(), 
            indices.size_bytes(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        Memory::Buffer::ScheduleUploading(
            mResourceFactory, 
            mResourceCopyManager, 
            mesh.vertexCoordinatesBuffer.get(),
            verticesCoordinates.data(),
            verticesCoordinates.size_bytes(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        Memory::Buffer::ScheduleUploading(
            mResourceFactory,
            mResourceCopyManager,
            mesh.vertexPropertiesBuffer.get(),
            verticesProperties.data(),
            verticesProperties.size_bytes(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        return mesh;
    }

    Engine::Scene::SceneData SceneToGPULoader::CreateSceneData(Context& context, const SceneDataDto& sceneDataDto)
    {
        SceneData sceneData = {};

        sceneData.whiteTextureIndex = CreateTexture(context, ImageLoader::CreateFromColor({ 1.f, 1.f, 1.f, 1.f }), "White");
        sceneData.blackTextureIndex = CreateTexture(context, ImageLoader::CreateFromColor({ 0.f, 0.f, 0.f, 1.f }), "Black");
        sceneData.fuchsiaTextureIndex = CreateTexture(context, ImageLoader::CreateFromColor({ 1.f, 0.f, 1.f, 1.f }), "Fuchsia");


        if (std::filesystem::exists(sceneDataDto.skyBoxPath))
        {
            auto image = ImageLoader::LoadImageFromFile(sceneDataDto.skyBoxPath);
            sceneData.skyBoxTextureIndex = CreateTexture(context, image, sceneDataDto.skyBoxPath);
        }

        return sceneData;
    }
}
