#pragma once

#include <Bin3D/Vertex.h>
#include <Bin3D/Meshlet.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>

namespace SceneConverter::Model
{
    struct RawMesh
    {
        uint32_t MaterialIndex;

        std::vector<uint32_t> Indices;
        std::vector<Bin3D::VertexCoordinates> Vertices;
        std::vector<Bin3D::VertexProperties> VertexProperties;

        DirectX::BoundingBox AABB;
    };

    struct RawMeshletData
    {
        std::vector<Bin3D::Meshlet> Meshlets;
        std::vector<Bin3D::MeshletTriangle> PrimitiveIndices;
        std::vector<uint32_t> UniqueVertexIB;
    };
}