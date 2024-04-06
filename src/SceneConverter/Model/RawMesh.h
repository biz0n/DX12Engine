#pragma once

#include <Bin3D/Vertex.h>
#include <Bin3D/Meshlet.h>

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <vector>

namespace SceneConverter::Model
{
    struct RawVertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TextureCoord;
        DirectX::XMFLOAT4 Tangent;

        Bin3D::VertexCoordinates GetVertexCoordinates() const
        {
            Bin3D::VertexCoordinates coords;
            coords.Position = Position;
            return coords;
        }

        Bin3D::VertexProperties GetVertexProperties() const
        {
            Bin3D::VertexProperties properties;
            properties.Normal = Normal;
            properties.Tangent = Tangent;
            properties.TextureCoord = TextureCoord;
            return properties;
        }
    };

    struct RawMesh
    {
        uint32_t MaterialIndex;

        std::vector<uint32_t> Indices;
        std::vector<RawVertex> Vertices;

        DirectX::BoundingBox AABB;
    };

    struct RawMeshletData
    {
        std::vector<Bin3D::Meshlet> Meshlets;
        std::vector<Bin3D::MeshletTriangle> PrimitiveIndices;
        std::vector<uint32_t> UniqueVertexIB;
    };
}