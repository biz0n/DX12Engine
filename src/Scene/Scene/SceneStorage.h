#pragma once

#include <Scene/Sampler.h>
#include <Scene/Camera.h>
#include <Scene/Material.h>
#include <Scene/PunctualLight.h>
#include <Scene/Vertex.h>
#include <Scene/Mesh.h>
#include <Scene/Node.h>
#include <Scene/ImagePath.h>
#include <Scene/DataRegion.h>

#include <vector>
#include <span>
#include <string_view>

namespace Engine::Scene
{
    class SceneStorage
    {
    public:
        SceneStorage(std::vector<char>&& dataVector);

        const std::span<const Engine::Scene::Node>& GetNodes() const { return mNodes; }
        const std::span<const Engine::Scene::Mesh>& GetMeshes() const { return mMeshes; }
        const std::span<const Engine::Scene::Material>& GetMaterials() const { return mMaterials; }
        const std::span<const Engine::Scene::PunctualLight>& GetLights() const { return mLights; }
        const std::span<const Engine::Scene::Camera>& GetCameras() const { return mCameras; }
        const std::span<const Engine::Scene::Sampler>& GetSamplers() const { return mSamplers; }

        std::span<const uint32_t> GetMeshIndices(const DataRegion& range) const;
        std::span<const Engine::Scene::Vertex> GetVertices(const DataRegion& range) const;
        std::span<const uint32_t> GetIndices(const DataRegion& range) const;

        std::string_view GetImageName(uint32_t index) const;
        std::string_view GetString(const DataRegion& range) const;

    private:
        template <typename T>
        std::span<const T> GetSpan(const char* data, const DataRegion& region)
        {
            return std::span{ reinterpret_cast<const T*>(data + region.Offset), region.Size };
        }

        std::vector<char> mDataVector;

        std::span<const Engine::Scene::Node> mNodes;
        std::span<const Engine::Scene::Mesh> mMeshes;
        std::span<const Engine::Scene::Material> mMaterials;
        std::span<const Engine::Scene::PunctualLight> mLights;
        std::span<const Engine::Scene::Camera> mCameras;
        std::span<const Engine::Scene::Sampler> mSamplers;
        
        std::span<const uint32_t> mNodeMeshIndicesStorage;
        std::span<const Engine::Scene::Vertex> mVerticesStorage;
        std::span<const uint32_t> mIndicesStorage;
        std::span<const Engine::Scene::ImagePath> mImagePaths;
        std::span<const char> mStringsStorage;
    };

}

