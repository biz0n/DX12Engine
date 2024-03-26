#include "MeshProcessor.h"

#include <spdlog/spdlog.h>

#include <DirectXMesh.h>
#include <meshoptimizer.h>



namespace SceneConverter::Model
{
    RawMeshletData MeshProcessor::GenerateMeshlet(const RawMesh& mesh)
    {
        std::span<const DirectX::XMFLOAT3> vertices = std::span{ reinterpret_cast<const DirectX::XMFLOAT3*>(mesh.Vertices.data()), mesh.Vertices.size() };
        std::span<const uint32_t> indices = std::span{ mesh.Indices.begin(), mesh.Indices.end() };

        return MeshOptimizerGenerateMeshlet(vertices, indices);
    }

    RawMeshletData MeshProcessor::MeshOptimizerGenerateMeshlet(const std::span<const DirectX::XMFLOAT3>& vertices, const std::span<const uint32_t>& indices)
    {
        RawMeshletData meshletData = {};

        // https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
        constexpr size_t maxVertices = 64;
        constexpr size_t maxTriangles = 124;
        constexpr float coneWeight = 0.0f;


        size_t max_meshlets = meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles);
        std::vector<meshopt_Meshlet> meshlets(max_meshlets);
        std::vector<unsigned int> meshletVertices(max_meshlets * maxVertices);
        std::vector<unsigned char> meshletTriangles(max_meshlets * maxTriangles * 3);

        size_t meshletCount = meshopt_buildMeshlets(
            meshlets.data(), 
            meshletVertices.data(), 
            meshletTriangles.data(), 
            indices.data(),
            indices.size(), 
            &vertices[0].x, 
            vertices.size(), 
            sizeof(DirectX::XMFLOAT3), 
            maxVertices, 
            maxTriangles, 
            coneWeight);

        const meshopt_Meshlet& last = meshlets[meshletCount - 1];
        size_t meshletVerticesCount = last.vertex_offset + last.vertex_count;

        int triangle_offset = 0;
        for (int i = 0; i < meshletCount; ++i)
        {
            auto& meshlet = meshlets.at(i);
            Bin3D::Meshlet m;
            m.PrimCount = meshlet.triangle_count;
            m.PrimOffset = triangle_offset;
            triangle_offset += meshlet.triangle_count;

            m.VertCount = meshlet.vertex_count;
            m.VertOffset = meshlet.vertex_offset;
            meshletData.Meshlets.push_back(m);

            for (int i = 0; i < meshlet.triangle_count * 3; i += 3)
            {
                Bin3D::MeshletTriangle t;
                t.i0 = meshletTriangles[meshlet.triangle_offset + i + 0];
                t.i1 = meshletTriangles[meshlet.triangle_offset + i + 1];
                t.i2 = meshletTriangles[meshlet.triangle_offset + i + 2];
                meshletData.PrimitiveIndices.push_back(t);
            }
        }

        for (int i = 0; i < meshletVerticesCount; ++i)
        {
            meshletData.UniqueVertexIB.push_back(meshletVertices[i]);
        }

        return meshletData;
    }

    RawMeshletData MeshProcessor::DirectXGenerateMeshlet(const std::span<const DirectX::XMFLOAT3>& vertices, const std::span<const uint32_t>& indices)
    {
        RawMeshletData meshletData = {};

        std::vector<DirectX::Meshlet> meshlets;
        std::vector<uint8_t> uniqueVertexIB;
        std::vector<DirectX::MeshletTriangle> primitiveIndices;

        // https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
        constexpr size_t meshletMaxVerts = 64;
        constexpr size_t meshletMaxPrimitives = 126;

        const auto nFaces = indices.size() / 3;

        HRESULT result = S_OK;

        result = DirectX::ComputeMeshlets(
            indices.data(), 
            nFaces,
            vertices.data(), 
            vertices.size(),
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
            meshletData.Meshlets.push_back(*reinterpret_cast<const Bin3D::Meshlet*>(&m));
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

        return meshletData;
    }
}
