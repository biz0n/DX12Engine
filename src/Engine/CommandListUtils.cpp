#include "CommandListUtils.h"

#include <Utils.h>
#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/UploadBuffer.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DynamicDescriptorHeap.h>

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

    void UploadVertexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> resourceTracker, VertexBuffer &vertexBuffer, CommandListContext &commandListContext)
    {
        UploadBuffer(device, commandList, resourceTracker, vertexBuffer, commandListContext);
    }

    void UploadIndexBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> resourceTracker, IndexBuffer &indexBuffer, CommandListContext &commandListContext)
    {
        UploadBuffer(device, commandList, resourceTracker, indexBuffer, commandListContext);
    }

    void UploadBuffer(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> resourceTracker, Buffer &buffer, CommandListContext &commandListContext, D3D12_RESOURCE_FLAGS flags)
    {
        Size bufferSize = buffer.GetElementsCount() * buffer.GetElementSize();

        ComPtr<ID3D12Resource> destinationResource;
        ComPtr<ID3D12Resource> intermediateResource;
        ThrowIfFailed(device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
            D3D12_RESOURCE_STATE_COMMON,
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

        resourceTracker->TrackResource(destinationResource.Get(), D3D12_RESOURCE_STATE_COMMON);
        TransitionBarrier(commandList, resourceTracker, destinationResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, true);

        UpdateSubresources(commandList.Get(), destinationResource.Get(), intermediateResource.Get(), 0, 0, 1, &subresource);

        
        buffer.SetD3D12Resource(destinationResource);
        buffer.CreateViews();

        commandListContext.TrackResource(destinationResource);
        commandListContext.TrackResource(intermediateResource);
    }

    void UploadTexture(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> resourceTracker, Scene::Texture &texture, CommandListContext &commandListContext)
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
            auto &metadata = image->GetImage()->GetMetadata();
            const DirectX::Image *images = image->GetImage()->GetImages();
            Size imageCount = image->GetImage()->GetImageCount();

            ComPtr<ID3D12Resource> textureResource;
            ComPtr<ID3D12Resource> intermadiateResource;

            DXGI_FORMAT format = metadata.format;
            if (texture.IsSRGB())
            {
                format = DirectX::MakeSRGB(format);
            }

            D3D12_RESOURCE_DESC desc = {};
            desc.Width = static_cast<UINT>(metadata.width);
            desc.Height = static_cast<UINT>(metadata.height);
            desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
            desc.DepthOrArraySize = (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
                ? static_cast<UINT16>(metadata.depth)
                : static_cast<UINT16>(metadata.arraySize);
            desc.Format = format;
            desc.Flags = D3D12_RESOURCE_FLAG_NONE;
            desc.SampleDesc.Count = 1;
            desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

            ThrowIfFailed(device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(&textureResource)));

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

            resourceTracker->TrackResource(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);
            TransitionBarrier(commandList, resourceTracker, textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, true);

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
        }
    }

    void UploadMaterialTextures(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> resourceTracker, CommandListContext &commandListContext, SharedPtr<Scene::Material> material)
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

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, const VertexBuffer &vertexBuffer)
    {
        TransitionBarrier(stateTracker, vertexBuffer.GetD3D12Resource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        commandList->IASetVertexBuffers(0, 1, &vertexBuffer.GetVertexBufferView());
    }

    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, const IndexBuffer &indexBuffer)
    {
        TransitionBarrier(stateTracker, indexBuffer.GetD3D12Resource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

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
        uniform.Cutoff = properties.alphaCutoff;

        uniform.HasBaseColorTexture = material->HasAlbedoTexture();
        uniform.HasNormalTexture = material->HasNormalTexture();
        uniform.HasMetallicRoughnessTexture = material->HasMetallicRoughnessTexture();
        uniform.HasOcclusionTexture = material->HasAmbientOcclusionTexture();
        uniform.HasEmissiveTexture = material->HasEmissiveTexture();

        return uniform;
    }

    void BindMaterial(ComPtr<ID3D12Device> device, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<DescriptorAllocator> descriptorAllocator, SharedPtr<Scene::Material> material)
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