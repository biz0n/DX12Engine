#pragma once

#include <Types.h>

#include <Memory/MemoryForwards.h>
#include <Scene/SceneForwards.h>
#include <Memory/UploadBuffer.h>
#include <Render/Shaders/ShaderTypes.h>
#include <Scene/Components/MeshComponent.h>

#include <entt/entt.hpp>
#include <vector>

namespace Engine::Scene
{
    class SceneStorage
    {
    public:
        void UploadSceneStructures(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer);

        const std::vector<Shader::MeshUniform>& GetMeshUniforms() const { return mMeshUniforms; }
        const Memory::UploadBuffer::Allocation& GetMeshUniformsAllocation() const { return mMeshUniformsAllocation; }
        const std::vector<Scene::Mesh>& GetMeshes() const { return mMeshes; }

        const std::vector<Material>& GetMaterials() const { return mMaterials; }
        const Memory::UploadBuffer::Allocation& GetMaterialUniformsAllocation() const { return mMaterialUniformsAllocation; }

        const Memory::UploadBuffer::Allocation& GetLightUniformsAllocation() const { return mLightUniformsAllocation; }

        int GetLightsCount() const { return mLightUniforms.size(); }
        bool HasDirectionalLight() const { return mHasDirectionalLight; }
    private:
        void UploadMeshes(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer);
        void UploadMaterials(SharedPtr<Memory::UploadBuffer> uploadBuffer);
        void UploadLights(const entt::registry& registry, SharedPtr<Memory::UploadBuffer> uploadBuffer);
    private:
        std::vector<SharedPtr<Memory::Texture>> mTextures;
        std::vector<Material> mMaterials;
        std::vector<Mesh> mMeshes;

        std::vector<Shader::MeshUniform> mMeshUniforms;
        std::vector<Shader::MaterialUniform> mMaterialUniforms;
        std::vector<Shader::LightUniform> mLightUniforms;
        bool mHasDirectionalLight;

        Memory::UploadBuffer::Allocation mMeshUniformsAllocation;
        Memory::UploadBuffer::Allocation mMaterialUniformsAllocation;
        Memory::UploadBuffer::Allocation mLightUniformsAllocation;

        friend Engine::Scene::SceneToRegisterLoader;
    }; 
}