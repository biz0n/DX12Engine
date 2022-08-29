#include "RenderRequest.h"

#include <Memory/UploadBuffer.h>
#include <Scene/SceneStorage.h>

namespace Engine::Render
{
    RenderRequest::RenderRequest(
        MeshPack&& meshes,
        std::vector<Shader::LightUniform>&& lights,
        std::vector<Shader::MaterialUniform>&& materials,
        RenderCamera&& camera,
        std::vector<RenderCamera>&& shadowCameras,
        SharedPtr<const Scene::SceneStorage> sceneStorage) :
        mMeshes{ meshes }, mLights{ lights }, mMaterials{materials}, mCamera {
        camera
    }, mShadowCameras{ shadowCameras }, mSceneStorage{ sceneStorage }
    {

    }

    void RenderRequest::UploadUniforms(SharedPtr<Memory::UploadBuffer> uploadBuffer)
    {
        mMeshUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::MeshUniform) * mMeshes.meshes.size());
        mMeshUniformsAllocation.CopyTo(mMeshes.meshes);

        mLightUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::LightUniform) * mLights.size());
        mLightUniformsAllocation.CopyTo(mLights);

        mMaterialUniformsAllocation = uploadBuffer->Allocate(sizeof(Shader::MaterialUniform) * mMaterials.size());
        mMaterialUniformsAllocation.CopyTo(mMaterials);
    }
}