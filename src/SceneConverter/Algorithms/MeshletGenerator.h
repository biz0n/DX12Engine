#pragma once

#include <Model/RawMesh.h>
#include <vector>

namespace SceneConverter::Algorithms::MeshletGenerator
{
    Model::RawMeshletData MeshOptimizerGenerateMeshlet(const std::vector<Model::RawVertex>& vertices, const std::vector<uint32_t>& indices);
    Model::RawMeshletData DirectXGenerateMeshlet(const std::vector<Model::RawVertex>& vertices, const std::vector<uint32_t>& indices);
}