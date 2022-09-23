#include "SceneStorage.h"

#include <Bin3D/Reader/BinaryHeader.h>

namespace Bin3D
{
    SceneStorage::SceneStorage(const std::filesystem::path& path, std::vector<char>&& dataVector) : mPath{path}, mDataVector { std::move(dataVector) }
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
        
        mVerticesCoordinatesStorage = GetSpan<VertexCoordinates>(data, header.VerticesCoordinatesStorage);
        mVerticesPropertiesStorage = GetSpan<VertexProperties>(data, header.VerticesPropertiesStorage);
        mIndicesStorage = GetSpan<uint32_t>(data, header.IndicesStorage);
        mImagePaths = GetSpan<ImagePath>(data, header.ImagePaths);
        mMeshlets = GetSpan<Meshlet>(data, header.Meshlets);
        mPrimitiveIndices = GetSpan<MeshletTriangle>(data, header.PrimitiveIndices);
        mUniqueVertexIndices = GetSpan<uint8_t>(data, header.UniqueVertexIndices);
        mStringsStorage = GetSpan<char>(data, header.StringsStorage);
    }

    std::span<const Bin3D::VertexCoordinates> SceneStorage::GetVerticesCoordinates(const DataRegion& range) const
    {
        return mVerticesCoordinatesStorage.subspan(range.Offset, range.Size);
    }

    std::span<const Bin3D::VertexProperties> SceneStorage::GetVerticesProperties(const DataRegion& range) const
    {
        return mVerticesPropertiesStorage.subspan(range.Offset, range.Size);
    }

    std::span<const uint32_t> SceneStorage::GetIndices(const DataRegion& range) const
    {
        return mIndicesStorage.subspan(range.Offset, range.Size);
    }

    std::span<const Bin3D::Meshlet> SceneStorage::GetMeshlets(const DataRegion& range) const
    {
        return mMeshlets.subspan(range.Offset, range.Size);
    }

    std::span<const Bin3D::MeshletTriangle> SceneStorage::GetPrimitiveIndices(const DataRegion& range) const
    {
        return mPrimitiveIndices.subspan(range.Offset, range.Size);
    }

    std::span<const uint8_t> SceneStorage::GetUniqueVertexIndices(const DataRegion& range) const
    {
        return mUniqueVertexIndices.subspan(range.Offset, range.Size);
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