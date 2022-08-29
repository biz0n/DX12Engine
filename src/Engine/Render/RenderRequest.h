#pragma once

#include <Types.h>
#include <Memory/Texture.h>
#include <Memory/UploadBuffer.h>
#include <Render/Shaders/ShaderTypes.h>
#include <Scene/SceneStorage.h>

#include <vector>
#include <ranges>

namespace Engine::Render
{
    struct MeshPack
    {
        std::vector<Shader::MeshUniform> meshes;
        std::ranges::iota_view<Index, Index> opaque;
    };

    struct RenderCamera
    {
        dx::XMMATRIX projection;
        dx::XMMATRIX view;
        dx::XMMATRIX viewProjection;
        dx::XMVECTOR eyePosition;
    };

    class RenderRequest
    {
    public:
        RenderRequest(
            MeshPack&& meshes, 
            std::vector<Shader::LightUniform>&& lights,
            std::vector<Shader::MaterialUniform>&& materials,
            RenderCamera&& camera,
            std::vector<RenderCamera>&& shadowCameras,
            SharedPtr<const Scene::SceneStorage> sceneStorage);

        ~RenderRequest() = default;

        int GetLightsCount() const { return mLights.size(); }

        const MeshPack& GetMeshes() const { return mMeshes; }
        const std::vector<Shader::MaterialUniform>& GetMaterials() const { return mMaterials; }
        const std::vector<Shader::LightUniform>& GetLights() const { return mLights; }
        const RenderCamera& GetCamera() const { return mCamera; }
        const std::vector<RenderCamera>& GetShadowCameras() const { return mShadowCameras; }

        const Memory::UploadBuffer::Allocation& GetMeshAllocation() const { return mMeshUniformsAllocation; }
        const Memory::UploadBuffer::Allocation& GetMaterialAllocation() const { return mMaterialUniformsAllocation; }
        const Memory::UploadBuffer::Allocation& GetLightAllocation() const { return mLightUniformsAllocation; }


        SharedPtr<const Scene::SceneStorage> GetSceneStorage() const { return mSceneStorage; }

        void UploadUniforms(SharedPtr<Memory::UploadBuffer> uploadBuffer);
    private:
        MeshPack mMeshes;
        std::vector<Shader::MaterialUniform> mMaterials;
        std::vector<Shader::LightUniform> mLights;
        RenderCamera mCamera;
        std::vector<RenderCamera> mShadowCameras;

        Memory::UploadBuffer::Allocation mMeshUniformsAllocation;
        Memory::UploadBuffer::Allocation mMaterialUniformsAllocation;
        Memory::UploadBuffer::Allocation mLightUniformsAllocation;

        SharedPtr<const Scene::SceneStorage> mSceneStorage;
    };
}