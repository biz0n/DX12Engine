#pragma once

#include <Bin3D/Sampler.h>
#include <Bin3D/Camera.h>
#include <Bin3D/Material.h>
#include <Bin3D/PunctualLight.h>
#include <Bin3D/Vertex.h>
#include <Bin3D/Mesh.h>
#include <Bin3D/Node.h>
#include <Bin3D/ImagePath.h>
#include <Bin3D/DataRegion.h>

#include <vector>
#include <span>
#include <string_view>
#include <filesystem>

namespace Bin3D
{
    class SceneStorage
    {
    public:
        SceneStorage(const std::filesystem::path& path, std::vector<char>&& dataVector);

        const std::filesystem::path& GetPath() const { return mPath; }

        const std::span<const Bin3D::Node>& GetNodes() const { return mNodes; }
        const std::span<const Bin3D::Mesh>& GetMeshes() const { return mMeshes; }
        const std::span<const Bin3D::Material>& GetMaterials() const { return mMaterials; }
        const std::span<const Bin3D::PunctualLight>& GetLights() const { return mLights; }
        const std::span<const Bin3D::Camera>& GetCameras() const { return mCameras; }
        const std::span<const Bin3D::Sampler>& GetSamplers() const { return mSamplers; }

        const std::span<const Bin3D::ImagePath>& GetImagePaths() const { return mImagePaths; }

        std::span<const uint32_t> GetMeshIndices(const DataRegion& range) const;
        std::span<const Bin3D::Vertex> GetVertices(const DataRegion& range) const;
        std::span<const uint32_t> GetIndices(const DataRegion& range) const;

        std::string_view GetImageName(uint32_t index) const;
        std::string_view GetString(const DataRegion& range) const;

    private:
        template <typename T>
        std::span<const T> GetSpan(const char* data, const DataRegion& region)
        {
            return std::span{ reinterpret_cast<const T*>(data + region.Offset), region.Size };
        }

        std::filesystem::path mPath;
        std::vector<char> mDataVector;

        std::span<const Bin3D::Node> mNodes;
        std::span<const Bin3D::Mesh> mMeshes;
        std::span<const Bin3D::Material> mMaterials;
        std::span<const Bin3D::PunctualLight> mLights;
        std::span<const Bin3D::Camera> mCameras;
        std::span<const Bin3D::Sampler> mSamplers;
        
        std::span<const uint32_t> mNodeMeshIndicesStorage;
        std::span<const Bin3D::Vertex> mVerticesStorage;
        std::span<const uint32_t> mIndicesStorage;
        std::span<const Bin3D::ImagePath> mImagePaths;
        std::span<const char> mStringsStorage;
    };

}

