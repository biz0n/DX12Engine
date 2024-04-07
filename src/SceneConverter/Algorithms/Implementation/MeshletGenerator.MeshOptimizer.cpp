#include <Algorithms/MeshletGenerator.h>

#include <meshoptimizer.h>

namespace SceneConverter::Algorithms::MeshletGenerator
{
    Bin3D::CullData ConvertToCullData(const meshopt_Bounds& bounds);

    Model::RawMeshletData MeshOptimizerGenerateMeshlet(const std::vector<Model::RawVertex>& vertices, const std::vector<uint32_t>& indices)
    {
        Model::RawMeshletData meshletData = {};

        // https://developer.nvidia.com/blog/introduction-turing-mesh-shaders/
        constexpr size_t maxVertices = 64;
        constexpr size_t maxTriangles = 124;
        constexpr float coneWeight = 1.0f;


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
            const auto& meshlet = meshlets.at(i);
            Bin3D::Meshlet m;
            m.PrimCount = meshlet.triangle_count;
            m.PrimOffset = triangle_offset;
            triangle_offset += meshlet.triangle_count;

            m.VertCount = meshlet.vertex_count;
            m.VertOffset = meshlet.vertex_offset;
            
            meshopt_Bounds bounds = meshopt_computeMeshletBounds(
                meshletVertices.data() + meshlet.vertex_offset,
                meshletTriangles.data() + meshlet.triangle_offset,
                meshlet.triangle_count,
                &vertices[0].Position.x,
                vertices.size(),
                sizeof(Model::RawVertex));

            m.CullData = ConvertToCullData(bounds);

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

    Bin3D::CullData ConvertToCullData(const meshopt_Bounds& bounds)
    {
        Bin3D::CullData cullData = {};

        cullData.BoundingSphere = DirectX::BoundingSphere(DirectX::XMFLOAT3(bounds.center), bounds.radius);

        DirectX::XMVECTOR axis = DirectX::XMVectorSet(bounds.cone_axis[0], bounds.cone_axis[1], bounds.cone_axis[2], 0);
        DirectX::PackedVector::XMBYTEN4 snquant;
        DirectX::PackedVector::XMStoreByteN4(&snquant, axis);

        cullData.NormalCone.x = uint8_t(int16_t(snquant.x) + 128);
        cullData.NormalCone.y = uint8_t(int16_t(snquant.y) + 128);
        cullData.NormalCone.z = uint8_t(int16_t(snquant.z) + 128);

        DirectX::XMVECTOR coneCutoff = DirectX::XMVectorSet(bounds.cone_cutoff, 0, 0, 0);
        DirectX::PackedVector::XMUBYTEN4 nquant;
        DirectX::PackedVector::XMStoreUByteN4(&nquant, coneCutoff);

        cullData.NormalCone.w = nquant.x;

        float maxt = 0;
        for (int i = 0; i < 3; ++i)
        {
            if (bounds.cone_axis[i] == 0)
            {
                continue;
            }
            float t = (bounds.center[i] - bounds.cone_apex[i]) / bounds.cone_axis[i];
            maxt = (t > maxt) ? t : maxt;
        }
        cullData.ApexOffset = maxt;

        return cullData;
    }
}