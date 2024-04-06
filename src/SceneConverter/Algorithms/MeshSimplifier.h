#pragma once

#include <Model/RawMesh.h>
#include <vector>
#include <unordered_set>

namespace SceneConverter::Algorithms::MeshletSimplifier
{
    std::vector<uint32_t> MeshOptimizerSimplify(
        const std::vector<uint32_t>& indices,
        const std::vector<Model::RawVertex>& vertices,
        const std::unordered_set<uint32_t>& boundaries,
        float threshold,
        float targetError,
        float& resultError);

    std::vector<uint32_t> VCGLibSimplify(
        const std::vector<uint32_t>& indices,
        const std::vector<Model::RawVertex>& vertices,
        const std::unordered_set<uint32_t>& boundaries,
        float threshold,
        float targetError,
        float& resultError);
}