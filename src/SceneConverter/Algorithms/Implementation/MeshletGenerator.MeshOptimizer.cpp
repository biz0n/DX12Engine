#include <Algorithms/MeshletGenerator.h>

#include <meshoptimizer.h>

namespace SceneConverter::Algorithms::MeshletGenerator
{
    Model::RawMeshletData MeshOptimizerGenerateMeshlet(const std::vector<Model::RawVertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Model::RawMeshletData meshletData = {};

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
            &vertices[0].Position.x,
            vertices.size(),
            sizeof(Model::RawVertex),
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

            for (size_t i = 0; i < meshlet.triangle_count * 3; i += 3)
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
}