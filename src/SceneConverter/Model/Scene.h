#pragma once

#include <Scene/Sampler.h>
#include <Scene/Camera.h>
#include <Scene/Material.h>
#include <Scene/PunctualLight.h>
#include <Scene/Vertex.h>
#include <Scene/ArrayIndex.h>
#include <Scene/Mesh.h>
#include <Scene/Node.h>
#include <Scene/ImagePath.h>

#include <Model/ImageData.h>]
#include <Model/Node.h>

#include <vector>
#include <map>
#include <unordered_map>

namespace SceneConverter::Model
{
    class Scene
    {
        public:
            Scene();

            const std::vector<Engine::Scene::Node>& GetNodes() const { return mNodes; }
            const std::vector<Engine::Scene::Mesh>& GetMeshes() const { return mMeshes; };
            const std::vector<Engine::Scene::Material>& GetMaterials() const { return mMaterials; }
            const std::vector<Engine::Scene::PunctualLight>& GetLights() const { return mLights; }
            const std::vector<Engine::Scene::Camera>& GetCameras() const { return mCameras; }
            const std::vector<Engine::Scene::Sampler>& GetSamplers() const { return mSamplers; }

            const std::vector<uint32_t>& GetNodeMeshIndicesStorage() const { return mMeshIndicesStorage; }
            const std::vector<Engine::Scene::Vertex>& GetVerticesStorage() const { return mVerticesStorage; }
            const std::vector<uint32_t>& GetIndicesStorage() const { return mIndicesStorage; }
            const std::vector<Engine::Scene::ImagePath>& GetImagePaths() const { return mImagePaths; }
            const std::vector<char>& GetStringsStorage() const { return mStringsStorage; }

            Engine::Scene::Sampler GetDefaultSampler() const { return mSamplers[0]; }
            const std::vector<std::shared_ptr<ImageData>>& GetImageResources() const { return mImageResources; }

            void AddRootNode(const Node& node);
            uint32_t AddMesh(const Engine::Scene::Mesh& mesh);
            uint32_t AddMaterial(const Engine::Scene::Material& material);
            uint32_t AddLight(const Engine::Scene::PunctualLight& light);
            uint32_t AddCamera(const Engine::Scene::Camera& camera);
            uint32_t AddSampler(const Engine::Scene::Sampler& sampler);

            Engine::Scene::ArrayIndex AddNodeMeshIndices(const std::vector<uint32_t>& indices);
            Engine::Scene::ArrayIndex AddIndices(const std::vector<uint32_t>& indices);
            Engine::Scene::ArrayIndex AddVertices(const std::vector<Engine::Scene::Vertex>& vertices);
            
            Engine::Scene::ArrayIndex AddString(const std::string& str);

            uint32_t AddImage(std::shared_ptr<ImageData> image);

            void UnwrapNodeTree();
            void FulfillImagePaths();

        private:
            std::vector<Engine::Scene::Mesh> mMeshes;
            std::vector<Engine::Scene::Material> mMaterials;
            std::vector<Engine::Scene::PunctualLight> mLights;
            std::vector<Engine::Scene::Camera> mCameras;
            std::vector<Engine::Scene::Sampler> mSamplers;
            std::vector<Engine::Scene::Node> mNodes;
            std::vector<Engine::Scene::ImagePath> mImagePaths;

            std::vector<std::shared_ptr<ImageData>> mImageResources;

            std::vector<Node> mRootNodes;
            
            std::vector<uint32_t> mMeshIndicesStorage;
            std::vector<Engine::Scene::Vertex> mVerticesStorage;
            std::vector<uint32_t> mIndicesStorage;
            std::vector<char> mStringsStorage;

            std::unordered_map<std::string, Engine::Scene::ArrayIndex> mStringsMap;
            std::map<Engine::Scene::Sampler, uint32_t> mSamplersMap;
    };
}