#include <Algorithms/MeshletGenerator.h>
#include <DirectXMesh.h>

#include <spdlog/spdlog.h>

namespace SceneConverter::Algorithms::MeshletGenerator
{
    Model::RawMeshletData DirectXGenerateMeshlet(const std::vector<Model::RawVertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Model::RawMeshletData meshletData = {};

        std::vector<DirectX::Meshlet> meshlets;
        std::vector<uint8_t> uniqueVertexIB;
        std::vector<DirectX::MeshletTriangle> primitiveIndices;
        
        // https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
        constexpr size_t meshletMaxVerts = 64;
        constexpr size_t meshletMaxPrimitives = 126;

        const auto nFaces = indices.size() / 3;

        HRESULT result = S_OK;

        std::vector<DirectX::XMFLOAT3> vertexPositions;
        vertexPositions.resize(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i)
        {
            vertexPositions[i] = vertices[i].Position;
        }

        result = DirectX::ComputeMeshlets(
            indices.data(),
            nFaces,
            vertexPositions.data(),
            vertexPositions.size(),
            nullptr,
            meshlets,
            uniqueVertexIB,
            primitiveIndices,
            meshletMaxVerts,
            meshletMaxPrimitives);

        if (FAILED(result))
        {
            spdlog::error("ComputeMeshlets Failed: {}", result);
        }

        for (const auto& m : meshlets)
        {
            Bin3D::Meshlet meshlet = {};
            meshlet.PrimCount = m.PrimCount;
            meshlet.PrimOffset = m.PrimOffset;
            meshlet.VertCount = m.VertCount;
            meshlet.VertOffset = m.VertOffset;
            meshletData.Meshlets.push_back(meshlet);
        }

        for (const auto& meshletTriangle : primitiveIndices)
        {
            meshletData.PrimitiveIndices.push_back(*reinterpret_cast<const Bin3D::MeshletTriangle*>(&meshletTriangle));
        }

        const uint32_t* indexBuffer = reinterpret_cast<const uint32_t*>(uniqueVertexIB.data());

        for (size_t i = 0; i < uniqueVertexIB.size() / sizeof(uint32_t); ++i)
        {
            meshletData.UniqueVertexIB.push_back(indexBuffer[i]);
        }

        std::vector<DirectX::CullData> cullData;
        cullData.resize(meshlets.size());
        result = DirectX::ComputeCullData(
            vertexPositions.data(),
            vertexPositions.size(),
            meshlets.data(),
            meshlets.size(),
            meshletData.UniqueVertexIB.data(),
            meshletData.UniqueVertexIB.size(),
            primitiveIndices.data(),
            primitiveIndices.size(),
            cullData.data());

        if (FAILED(result))
        {
            spdlog::error("ComputeCullData Failed: {}", result);
        }

        for (size_t i = 0; i < meshletData.Meshlets.size(); ++i)
        {
            meshletData.Meshlets[i].CullData = *reinterpret_cast<const Bin3D::CullData*>(&cullData[i]);
        }

        return meshletData;
    }
}