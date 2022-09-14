#pragma once

#include <Bin3D/Sampler.h>
#include <Bin3D/Camera.h>
#include <Bin3D/Material.h>
#include <Bin3D/PunctualLight.h>
#include <Bin3D/Vertex.h>
#include <Bin3D/DataRegion.h>
#include <Bin3D/Mesh.h>
#include <Bin3D/Node.h>
#include <Bin3D/ImagePath.h>

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

            const std::vector<Bin3D::Node>& GetNodes() const { return mNodes; }
            const std::vector<Bin3D::Mesh>& GetMeshes() const { return mMeshes; };
            const std::vector<Bin3D::Material>& GetMaterials() const { return mMaterials; }
            const std::vector<Bin3D::PunctualLight>& GetLights() const { return mLights; }
            const std::vector<Bin3D::Camera>& GetCameras() const { return mCameras; }
            const std::vector<Bin3D::Sampler>& GetSamplers() const { return mSamplers; }

            const std::vector<uint32_t>& GetNodeMeshIndicesStorage() const { return mMeshIndicesStorage; }
            const std::vector<Bin3D::Vertex>& GetVerticesStorage() const { return mVerticesStorage; }
            const std::vector<uint32_t>& GetIndicesStorage() const { return mIndicesStorage; }
            const std::vector<Bin3D::ImagePath>& GetImagePaths() const { return mImagePaths; }
            const std::vector<char>& GetStringsStorage() const { return mStringsStorage; }

            Bin3D::Sampler GetDefaultSampler() const { return mSamplers[0]; }
            const std::vector<std::shared_ptr<ImageData>>& GetImageResources() const { return mImageResources; }

            void AddRootNode(const Node& node);
            uint32_t AddMesh(const Bin3D::Mesh& mesh);
            uint32_t AddMaterial(const Bin3D::Material& material);
            uint32_t AddLight(const Bin3D::PunctualLight& light);
            uint32_t AddCamera(const Bin3D::Camera& camera);
            uint32_t AddSampler(const Bin3D::Sampler& sampler);

            Bin3D::DataRegion AddNodeMeshIndices(const std::vector<uint32_t>& indices);
            Bin3D::DataRegion AddIndices(const std::vector<uint32_t>& indices);
            Bin3D::DataRegion AddVertices(const std::vector<Bin3D::Vertex>& vertices);
            
            Bin3D::DataRegion AddString(const std::string& str);

            uint32_t AddImage(std::shared_ptr<ImageData> image);

            void UnwrapNodeTree();
            void FulfillImagePaths();

        private:
            std::vector<Bin3D::Mesh> mMeshes;
            std::vector<Bin3D::Material> mMaterials;
            std::vector<Bin3D::PunctualLight> mLights;
            std::vector<Bin3D::Camera> mCameras;
            std::vector<Bin3D::Sampler> mSamplers;
            std::vector<Bin3D::Node> mNodes;
            std::vector<Bin3D::ImagePath> mImagePaths;

            std::vector<std::shared_ptr<ImageData>> mImageResources;

            std::vector<Node> mRootNodes;
            
            std::vector<uint32_t> mMeshIndicesStorage;
            std::vector<Bin3D::Vertex> mVerticesStorage;
            std::vector<uint32_t> mIndicesStorage;
            std::vector<char> mStringsStorage;

            std::unordered_map<std::string, Bin3D::DataRegion> mStringsMap;
            std::map<Bin3D::Sampler, uint32_t> mSamplersMap;
    };
}