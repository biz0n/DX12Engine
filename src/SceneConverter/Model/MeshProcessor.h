#pragma once

#include <Model/RawMesh.h>

#include <span>

namespace SceneConverter::Model
{
    class MeshProcessor
    {
    public:
        RawMeshletData GenerateMeshlet(const RawMesh& mesh);
    private:
        RawMeshletData MeshOptimizerGenerateMeshlet(const std::span<const DirectX::XMFLOAT3>& vertices, const std::span<const uint32_t>& indices);

        RawMeshletData DirectXGenerateMeshlet(const std::span<const DirectX::XMFLOAT3>& vertices, const std::span<const uint32_t>& indices);
    };
}
