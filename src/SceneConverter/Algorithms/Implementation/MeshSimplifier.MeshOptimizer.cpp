#include <Algorithms/MeshSimplifier.h>

#include <meshoptimizer.h>
#include <Hash.h>

namespace SceneConverter::Algorithms::MeshletSimplifier
{
    std::vector<uint32_t> MeshOptimizerSimplify(
        const std::vector<uint32_t>& indices,
        const std::vector<Model::RawVertex>& vertices,
        const std::unordered_set<uint32_t>& boundaries,
        float threshold,
        float targetError,
        float& resultError)
    {
        size_t targetIndexCount = size_t(indices.size() * threshold);

        unsigned int options = meshopt_SimplifyLockBorder;

        std::vector<unsigned char> vertexLocks;
        vertexLocks.resize(vertices.size());
        for (size_t i = 0; i < indices.size(); ++i)
        {
             vertexLocks[indices[i]] = boundaries.find(indices[i]) != boundaries.end();
        }

        std::vector<uint32_t> lod(indices.size());
        size_t indicesCount = meshopt_simplifyWithAttributes(
            &lod[0],
            indices.data(),
            indices.size(),
            &vertices[0].Position.x,
            vertices.size(),
            sizeof(Model::RawVertex),
            NULL, 0, NULL, 0,
            NULL,//vertexLocks.data(),
            targetIndexCount,
            targetError,
            options,
            &resultError);

        lod.resize(indicesCount);

        return lod;
    }
}