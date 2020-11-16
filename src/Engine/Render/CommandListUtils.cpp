#include "CommandListUtils.h"

#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/UploadBuffer.h>
#include <Memory/DescriptorAllocator.h>
#include <Memory/DescriptorAllocation.h>
#include <Memory/DynamicDescriptorHeap.h>
#include <Render/RenderContext.h>

#include <Scene/Loader/SceneLoader.h>
#include <Scene/Image.h>
#include <Scene/Texture.h>
#include <Scene/Material.h>
#include <Memory/UploadBuffer.h>
#include <Scene/PunctualLight.h>
#include <Scene/Camera.h>

#include <DirectXTex.h>
#include <DirectXMath.h>

#include <filesystem>
#include <map>

namespace Engine::CommandListUtils
{
    void UploadVertexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, VertexBuffer &vertexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer)
    {
        UploadBuffer(renderContext, commandList, stateTracker, vertexBuffer, uploadBuffer);
    }

    void UploadIndexBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, IndexBuffer &indexBuffer, SharedPtr<Engine::UploadBuffer> uploadBuffer)
    {
        UploadBuffer(renderContext, commandList, stateTracker, indexBuffer, uploadBuffer);
    }

    void UploadBuffer(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Buffer &buffer, SharedPtr<Engine::UploadBuffer> uploadBuffer, D3D12_RESOURCE_FLAGS flags)
    {
        auto device = renderContext->Device();
        auto resourceTracker = stateTracker;
        Size bufferSize = buffer.GetElementsCount() * buffer.GetElementSize();

        ComPtr<ID3D12Resource> destinationResource;

        auto allocation = uploadBuffer->Allocate(bufferSize, 1U);
        CD3DX12_HEAP_PROPERTIES props{D3D12_HEAP_TYPE_DEFAULT};
        auto bufferSesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
        ThrowIfFailed(device->CreateCommittedResource(
            &props,
            D3D12_HEAP_FLAG_NONE,
            &bufferSesc,
            D3D12_RESOURCE_STATE_COMMON,
            nullptr,
            IID_PPV_ARGS(&destinationResource)));

        D3D12_SUBRESOURCE_DATA subresource;
        subresource.pData = buffer.GetData();
        subresource.SlicePitch = bufferSize;
        subresource.RowPitch = bufferSize;

        resourceTracker->TrackResource(destinationResource.Get(), D3D12_RESOURCE_STATE_COMMON);
        TransitionBarrier(commandList, resourceTracker, destinationResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, true);

        UpdateSubresources(commandList.Get(), destinationResource.Get(), uploadBuffer->GetD3D12Resource(), allocation.offset, 0, 1, &subresource);

        
        buffer.SetD3D12Resource(destinationResource);
    }

    void UploadTexture(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, Scene::Texture *texture, SharedPtr<Engine::UploadBuffer> uploadBuffer)
    {
        if (texture->GetD3D12Resource() != nullptr)
        {
            return;
        }

        auto device = renderContext->Device();
        auto resourceTracker = stateTracker;

        std::filesystem::path filename = texture->GetName();

        auto image = texture->GetImage();
        auto &metadata = image->GetImage()->GetMetadata();
        const DirectX::Image *images = image->GetImage()->GetImages();
        Size imageCount = image->GetImage()->GetImageCount();

        ComPtr<ID3D12Resource> textureResource;

        DXGI_FORMAT format = metadata.format;
        if (texture->IsSRGB())
        {
            format = DirectX::MakeSRGB(format);
        }

        D3D12_RESOURCE_DESC desc = {};
        desc.Width = static_cast<uint32>(metadata.width);
        desc.Height = static_cast<uint32>(metadata.height);
        desc.MipLevels = static_cast<uint16>(metadata.mipLevels);
        desc.DepthOrArraySize = (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D)
            ? static_cast<uint16>(metadata.depth)
            : static_cast<uint16>(metadata.arraySize);
        desc.Format = format;
        desc.Flags = D3D12_RESOURCE_FLAG_NONE;
        desc.SampleDesc.Count = 1;
        desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
        CD3DX12_HEAP_PROPERTIES props{D3D12_HEAP_TYPE_DEFAULT};
        ThrowIfFailed(device->CreateCommittedResource(
            &props,
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

        const uint64 uploadBufferSize = GetRequiredIntermediateSize(
            textureResource.Get(),
            0,
            static_cast<unsigned int>(subresources.size()));

        auto allocation = uploadBuffer->Allocate(uploadBufferSize, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);

        resourceTracker->TrackResource(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);
        TransitionBarrier(commandList, resourceTracker, textureResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, true);

        UpdateSubresources(
            commandList.Get(),
            textureResource.Get(),
            uploadBuffer->GetD3D12Resource(),
            allocation.offset,
            0,
            static_cast<unsigned int>(subresources.size()),
            subresources.data());

        texture->SetD3D12Resource(textureResource);
    }

    bool UploadMaterialTextures(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<Scene::Material> material, SharedPtr<Engine::UploadBuffer> uploadBuffer)
    {
        bool anythingToLoad = false;
        if (material->HasBaseColorTexture() && !material->GetBaseColorTexture()->GetD3D12Resource())
        {
            anythingToLoad = true;
            UploadTexture(renderContext, commandList, stateTracker, material->GetBaseColorTexture().get(), uploadBuffer);
        }

        if (material->HasMetallicRoughnessTexture() && !material->GetMetallicRoughnessTexture()->GetD3D12Resource())
        {
            anythingToLoad = true;
            UploadTexture(renderContext, commandList, stateTracker, material->GetMetallicRoughnessTexture().get(), uploadBuffer);
        }

        if (material->HasNormalTexture() && !material->GetNormalTexture()->GetD3D12Resource())
        {
            anythingToLoad = true;
            UploadTexture(renderContext, commandList, stateTracker, material->GetNormalTexture().get(), uploadBuffer);
        }

        if (material->HasEmissiveTexture() && !material->GetEmissiveTexture()->GetD3D12Resource())
        {
            anythingToLoad = true;
            UploadTexture(renderContext, commandList, stateTracker, material->GetEmissiveTexture().get(), uploadBuffer);
        }

        if (material->HasAmbientOcclusionTexture() && !material->GetAmbientOcclusionTexture()->GetD3D12Resource())
        {
            anythingToLoad = true;
            UploadTexture(renderContext, commandList, stateTracker, material->GetAmbientOcclusionTexture().get(), uploadBuffer);
        }

        return anythingToLoad;
    }

    void BindVertexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, VertexBuffer &vertexBuffer)
    {
        TransitionBarrier(stateTracker, vertexBuffer.GetD3D12Resource(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        auto vbv = vertexBuffer.GetVertexBufferView();
        commandList->IASetVertexBuffers(0, 1, &vbv);
    }

    void BindIndexBuffer(ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, IndexBuffer &indexBuffer)
    {
        TransitionBarrier(stateTracker, indexBuffer.GetD3D12Resource(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

        auto ibv = indexBuffer.GetIndexBufferView();
        commandList->IASetIndexBuffer(&ibv);
    }

    LightUniform GetLightUniform(const Scene::PunctualLight& lightNode, const DirectX::XMMATRIX& world)
    {
        LightUniform light = {};

        light.LightType = lightNode.GetLightType();
        light.Enabled = lightNode.IsEnabled();
        auto color = lightNode.GetColor();
        light.Color = { color.x * lightNode.GetIntensity(), color.y * lightNode.GetIntensity(), color.z * lightNode.GetIntensity() };

        DirectX::XMStoreFloat3(&light.PositionWS,
                               DirectX::XMVector4Transform(
                               DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
                               world));

        DirectX::XMFLOAT3 direction = lightNode.GetDirection();
        DirectX::XMStoreFloat3(&light.DirectionWS,
                               DirectX::XMVector4Transform(
                               DirectX::XMVectorSet(direction.x, direction.y, direction.z, 0.0f),
                               world));

        light.ConstantAttenuation = lightNode.GetConstantAttenuation();
        light.LinearAttenuation = lightNode.GetLinearAttenuation();
        light.QuadraticAttenuation = lightNode.GetQuadraticAttenuation();
        light.InnerConeAngle = lightNode.GetInnerConeAngle();
        light.OuterConeAngle = lightNode.GetOuterConeAngle();

        return light;
    }

    MaterialUniform GetMaterialUniform(const Scene::Material& material)
    {
        Scene::MaterialProperties properties = material.GetProperties();
        MaterialUniform uniform = {};
        uniform.BaseColor = properties.baseColor.baseColor;
        uniform.EmissiveFactor = {properties.emissive.factor.x, properties.emissive.factor.y, properties.emissive.factor.z, 1.0f};
        uniform.MetallicFactor = properties.metallicRaughness.metallicFactor;
        uniform.RoughnessFactor = properties.metallicRaughness.roughnessFactor;
        uniform.NormalScale = properties.normalTextureInfo.scale;
        uniform.Ambient = {0.9f, 0.9f, 0.9f, 0.0f};
        uniform.Cutoff = properties.alphaCutoff;

        uniform.HasBaseColorTexture = material.HasBaseColorTexture();
        uniform.HasNormalTexture = material.HasNormalTexture();
        uniform.HasMetallicRoughnessTexture = material.HasMetallicRoughnessTexture();
        uniform.HasOcclusionTexture = material.HasAmbientOcclusionTexture();
        uniform.HasEmissiveTexture = material.HasEmissiveTexture();

        return uniform;
    }

    FrameUniform GetFrameUniform(const Scene::Camera& camera, const DirectX::XMMATRIX& world, float32 aspectRatio, uint32 lightsCount)
    {
        auto projectionMatrix = camera.GetProjectionMatrix(aspectRatio);
        static const dx::XMVECTOR up = dx::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        static const dx::XMVECTOR forward = dx::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

        dx::XMVECTOR sc;
        dx::XMVECTOR rt;
        dx::XMVECTOR tr;
        dx::XMMatrixDecompose(&sc, &rt, &tr, world);

        auto rotationMatrix = dx::XMMatrixRotationQuaternion(rt);
        auto lookAtDirection = dx::XMVector3Transform(
            forward,
            dx::XMMatrixRotationQuaternion(rt));

        using namespace DirectX;
        auto viewMatrix = dx::XMMatrixLookAtLH(tr, tr + lookAtDirection, up);

        DirectX::XMMATRIX mvpMatrix = DirectX::XMMatrixMultiply(viewMatrix, projectionMatrix);
        mvpMatrix = DirectX::XMMatrixTranspose(mvpMatrix);

        FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, mvpMatrix);

        dx::XMStoreFloat3(&cb.EyePos, tr);

        cb.LightsCount = lightsCount;

        return cb;
    }

    MeshUniform GetMeshUniform(const DirectX::XMMATRIX& world)
    {
        DirectX::XMMATRIX tWorld = DirectX::XMMatrixTranspose(world);
        auto d = DirectX::XMMatrixDeterminant(tWorld);
        DirectX::XMMATRIX tWorldInverseTranspose = DirectX::XMMatrixInverse(&d, tWorld);
        tWorldInverseTranspose = DirectX::XMMatrixTranspose(tWorldInverseTranspose);
        MeshUniform cb;
        DirectX::XMStoreFloat4x4(&cb.World, tWorld);
        DirectX::XMStoreFloat4x4(&cb.InverseTranspose, tWorldInverseTranspose);

        return cb;
    }

    void BindMaterial(SharedPtr<RenderContext> renderContext, ComPtr<ID3D12GraphicsCommandList> commandList, SharedPtr<ResourceStateTracker> stateTracker, SharedPtr<::Engine::UploadBuffer> buffer, SharedPtr<DynamicDescriptorHeap> dynamicDescriptorHeap, SharedPtr<Scene::Material> material)
    {
        auto device = renderContext->Device();
        auto descriptorAllocator = renderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        MaterialUniform uniform = CommandListUtils::GetMaterialUniform(*material.get());

        auto matAllocation = buffer->Allocate(sizeof(MaterialUniform));
        matAllocation.CopyTo(&uniform);
        commandList->SetGraphicsRootConstantBufferView(2, matAllocation.GPU);

        if (material->HasBaseColorTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetBaseColorTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(4, 0, 1, material->GetBaseColorTexture()->GetShaderResourceView(device, descriptorAllocator));
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

        
        if (material->HasEmissiveTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetEmissiveTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(7, 0, 1, material->GetEmissiveTexture()->GetShaderResourceView(device, descriptorAllocator));
        }

        if (material->HasAmbientOcclusionTexture())
        {
            CommandListUtils::TransitionBarrier(stateTracker, material->GetAmbientOcclusionTexture()->GetD3D12Resource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

            dynamicDescriptorHeap->StageDescriptor(8, 0, 1, material->GetAmbientOcclusionTexture()->GetShaderResourceView(device, descriptorAllocator));
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