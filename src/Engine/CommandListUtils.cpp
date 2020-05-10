#include "CommandListUtils.h"

#include <Utils.h>
#include <Exceptions.h>
#include <d3dx12.h>

#include <Scene/Loader/SceneLoader.h>
#include <Scene/Image.h>
#include <Scene/Texture.h>
#include <Scene/LightNode.h>
#include <Scene/Material.h>

#include <DirectXTex.h>
#include <DirectXMath.h>

#include <filesystem>
#include <map>

namespace Engine::CommandListUtils
{
    static std::map<std::wstring, ID3D12Resource *> gsTextureCache;

    void UploadVertexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, VertexBuffer &vertexBuffer, CommandListContext &commandListContext)
    {
        UploadBuffer(device, commandList, vertexBuffer, commandListContext);
    }

    void UploadIndexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, IndexBuffer &indexBuffer, CommandListContext &commandListContext)
    {
        UploadBuffer(device, commandList, indexBuffer, commandListContext);
    }

    void UploadBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, Buffer &buffer, CommandListContext &commandListContext, D3D12_RESOURCE_FLAGS flags)
    {
        Size bufferSize = buffer.GetElementsCount() * buffer.GetElementSize();

        ComPtr<ID3D12Resource> destinationResource;
        ComPtr<ID3D12Resource> intermediateResource;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&destinationResource)));

        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&intermediateResource)));

        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = buffer.GetData();
        subresource.SlicePitch = bufferSize;
        subresource.RowPitch = bufferSize;

        UpdateSubresources(commandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0, 0, 1, &subresource);

        buffer.SetD3D12Resource(destinationResource);
        buffer.CreateViews();

        commandListContext.TrackResource(destinationResource);
        commandListContext.TrackResource(intermediateResource);
    }

    void UploadTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<GlobalResourceStateTracker> resourceTracker, Scene::Texture &texture, CommandListContext &commandListContext)
    {
        std::filesystem::path filename = texture.GetName();

        auto iter = gsTextureCache.find(filename);
        if (iter != gsTextureCache.end())
        {
            texture.SetD3D12Resource(iter->second);
        }
        else
        {
            auto image = texture.GetImage();
            auto metadata = image->GetImage()->GetMetadata();
            const DirectX::Image *images = image->GetImage()->GetImages();
            Size imageCount = image->GetImage()->GetImageCount();

            ComPtr<ID3D12Resource> textureResource;
            ComPtr<ID3D12Resource> intermadiateResource;

            ThrowIfFailed(CreateTextureEx(
                device.Get(),
                metadata,
                D3D12_RESOURCE_FLAG_NONE,
                texture.IsSRGB(),
                &textureResource));

            std::vector<D3D12_SUBRESOURCE_DATA> subresources;
            ThrowIfFailed(PrepareUpload(
                device.Get(),
                images,
                imageCount,
                metadata,
                subresources));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(
                textureResource.Get(),
                0,
                static_cast<unsigned int>(subresources.size()));

            ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&intermadiateResource)));

            UpdateSubresources(
                commandList.Get(),
                textureResource.Get(),
                intermadiateResource.Get(),
                0, 0,
                static_cast<unsigned int>(subresources.size()),
                subresources.data());

            texture.SetD3D12Resource(textureResource);

            gsTextureCache[filename] = textureResource.Get();

            commandListContext.TrackResource(textureResource);
            commandListContext.TrackResource(intermadiateResource);

            resourceTracker->TrackResource(textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
        }
    }

    void UploadMaterialTextures(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<GlobalResourceStateTracker> resourceTracker, CommandListContext &commandListContext, SharedPtr<Scene::Material> material)
    {
        if (material->HasAlbedoTexture())
        {
            UploadTexture(device, commandList, resourceTracker, *material->GetAlbedoTexture(), commandListContext);
        }

        if (material->HasMetallicRoughnessTexture())
        {
            UploadTexture(device, commandList, resourceTracker, *material->GetMetallicRoughnessTexture(), commandListContext);
        }

        if (material->HasNormalTexture())
        {
            UploadTexture(device, commandList, resourceTracker, *material->GetNormalTexture(), commandListContext);
        }
    }

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const VertexBuffer &vertexBuffer)
    {
        commandList->IASetVertexBuffers(0, 1, &vertexBuffer.GetVertexBufferView());
    }

    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, const IndexBuffer &indexBuffer)
    {
        commandList->IASetIndexBuffer(&indexBuffer.GetIndexBufferView());
    }

    LightUniform GetLightUniform(const Scene::LightNode *lightNode)
    {
        LightUniform light = {};

        light.LightType = lightNode->GetLightType();
        light.Enabled = lightNode->IsEnabled();
        light.Color = lightNode->GetColor();

        auto world = lightNode->GetWorldTransform();

        DirectX::XMStoreFloat3(&light.PositionWS,
                               DirectX::XMVector4Transform(
                                   DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                                   world));

        DirectX::XMFLOAT3 direction = lightNode->GetDirection();
        DirectX::XMStoreFloat3(&light.DirectionWS,
                               DirectX::XMVector4Transform(
                                   DirectX::XMVectorSet(direction.x, direction.y, direction.z, 0.0f),
                                   world));

        light.ConstantAttenuation = lightNode->GetConstantAttenuation();
        light.LinearAttenuation = lightNode->GetLinearAttenuation();
        light.QuadraticAttenuation = lightNode->GetQuadraticAttenuation();
        light.InnerConeAngle = lightNode->GetInnerConeAngle();
        light.OuterConeAngle = lightNode->GetOuterConeAngle();

        return light;
    }

    MaterialUniform GetMaterialUniform(const Scene::Material *material)
    {
        Scene::MaterialProperties properties = material->GetProperties();
        MaterialUniform uniform = {};
        uniform.Diffuse = properties.albedo.baseColor;
        uniform.Emissive = {properties.emissive.factor.x, properties.emissive.factor.y, properties.emissive.factor.z, 1.0f};
        uniform.MetallicFactor = properties.metallicRaughness.metallicFactor;
        uniform.RoughnessFactor = properties.metallicRaughness.roughnessFactor;
        uniform.NormalScale = properties.normalTextureInfo.scale;
        uniform.Ambient = {0.9f, 0.9f, 0.9f, 0.0f};

        uniform.HasBaseColorTexture = material->HasAlbedoTexture();
        uniform.HasNormalTexture = material->HasNormalTexture();
        uniform.HasMetallicRoughnessTexture = material->HasMetallicRoughnessTexture();
        uniform.HasOcclusionTexture = material->HasAmbientOcclusionTexture();
        uniform.HasEmissiveTexture = material->HasEmissiveTexture();

        return uniform;
    }

    void BindMaterial(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, DescriptorAllocator *descriptorAllocator, SharedPtr<Scene::Material> material)
    {
        MaterialUniform uniform = CommandListUtils::GetMaterialUniform(material.get());

        auto matAllocation = buffer->Allocate(sizeof(MaterialUniform));
        matAllocation.CopyTo(&uniform);
        commandList->SetGraphicsRootConstantBufferView(2, matAllocation.GPU);

        if (material->HasAlbedoTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetAlbedoTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(4, 0, 1, material->GetAlbedoTexture()->GetShaderResourceView(device, descriptorAllocator));
        }

        if (material->HasMetallicRoughnessTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetMetallicRoughnessTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(5, 0, 1, material->GetMetallicRoughnessTexture()->GetShaderResourceView(device, descriptorAllocator));
        }

        if (material->HasNormalTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetNormalTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(6, 0, 1, material->GetNormalTexture()->GetShaderResourceView(device, descriptorAllocator));
        }
    }

    void TransitionBarrier(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState, bool forceFlush)
    {
        TransitionBarrier(stateTracker, resource, targetState);

        if (forceFlush)
        {
            stateTracker->FlushBarriers(commandList);
        }
    }

    void TransitionBarrier(SharedPtr<ResourceStateTracker> stateTracker, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES targetState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource.Get(),
            D3D12_RESOURCE_STATE_COMMON,
            targetState);
        stateTracker->ResourceBarrier(barrier);
    }

} // namespace Engine::CommandListUtils