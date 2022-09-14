#include "SceneStorage.h"

#include <Scene/Reader/BinaryHeader.h>

namespace Engine::Scene
{
    SceneStorage::SceneStorage(std::vector<char>&& dataVector) : mDataVector{ std::move(dataVector) }
    {
        const auto* data = mDataVector.data();

        Reader::BinaryHeader header = {};
        std::memcpy(&header, data, sizeof(Reader::BinaryHeader));

        mNodes = GetSpan<Node>(data, header.Nodes);
        mMeshes = GetSpan<Mesh>(data, header.Meshes);
        mMaterials = GetSpan<Material>(data, header.Materials);
        mLights = GetSpan<PunctualLight>(data, header.Lights);
        mCameras = GetSpan<Camera>(data, header.Cameras);
        mSamplers = GetSpan<Sampler>(data, header.Samplers);
        
        mNodeMeshIndicesStorage = GetSpan<uint32_t>(data, header.NodeMeshIndicesStorage);
        mVerticesStorage = GetSpan<Vertex>(data, header.VerticesStorage);
        mIndicesStorage = GetSpan<uint32_t>(data, header.IndicesStorage);
        mImagePaths = GetSpan<ImagePath>(data, header.ImagePaths);
        mStringsStorage = GetSpan<char>(data, header.StringsStorage);
    }

    std::span<const uint32_t> SceneStorage::GetMeshIndices(const DataRegion& range) const
    {
        return mNodeMeshIndicesStorage.subspan(range.Offset, range.Size);
    }

    std::span<const Engine::Scene::Vertex> SceneStorage::GetVertices(const DataRegion& range) const
    {
        return mVerticesStorage.subspan(range.Offset, range.Size);
    }

    std::span<const uint32_t> SceneStorage::GetIndices(const DataRegion& range) const
    {
        return mIndicesStorage.subspan(range.Offset, range.Size);
    }

    std::string_view SceneStorage::GetImageName(uint32_t index) const
    {
        return GetString(mImagePaths[index].PathIndex);
    }

    std::string_view SceneStorage::GetString(const DataRegion& range) const
    {
        return std::string_view{ mStringsStorage.data() + range.Offset, range.Size };
    }
}