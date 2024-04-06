#include "MeshProcessor.h"

#include <spdlog/spdlog.h>

#include <unordered_set>
#include <vector>
#include <cmath>

#include <spdlog/spdlog.h>

#include <Algorithms/MeshletGenerator.h>
#include <Algorithms/MeshSimplifier.h>

#define IDXTYPEWIDTH 32
#define REALTYPEWIDTH 32
#include <metis.h>

#define GenerateMeshletFunc Algorithms::MeshletGenerator::MeshOptimizerGenerateMeshlet
//#define GenerateMeshletFunc Algorithms::MeshletGenerator::DirectXGenerateMeshlet

//#define MeshSimplifyFunc Algorithms::MeshletSimplifier::MeshOptimizerSimplify
#define MeshSimplifyFunc Algorithms::MeshletSimplifier::VCGLibSimplify


namespace SceneConverter::Model
{
    struct MeshletPair
    {
        uint32_t Meshlets[2] = {};
        uint32_t Count = 0;

        MeshletPair(uint32_t meshletId)
        {
            Meshlets[0] = meshletId;
            Count = 1;
        }

        bool AddMeshlet(uint32_t meshletIndex)
        {
            if (Count == 1 && Meshlets[0] != meshletIndex)
            {
                Meshlets[Count] = meshletIndex;
                Count++;

                return true;
            }

            return false;
            
        }

        uint32_t GetConnectedMeshlet(uint32_t meshletId) const
        {
            if (Meshlets[0] == meshletId)
            {
                return Meshlets[1];
            }
            else
            {
                return Meshlets[0];
            }
        }
    };

    struct Edge
    {
        uint32_t A;
        uint32_t B;

        Edge(uint32_t a, uint32_t b)
        {
            if (a < b)
            {
                A = a;
                B = b;
            }
            else
            {
                B = a;
                A = b;
            }
        }

        bool operator==(const Edge& other) const = default;
    };

    struct MeshletGroup
    {
        std::vector<uint32_t> Meshlets = {};
        std::unordered_set<uint32_t> BoundaryVertices = {};
    };
}


namespace std
{

    template <>
    struct hash<SceneConverter::Model::Edge>
    {
        size_t operator()(const SceneConverter::Model::Edge& key) const
        {
            return ((size_t)key.A << 32) | key.B;
        }
    };

} // namespace std


namespace SceneConverter::Model
{
    RawMeshletData CombineMeshlets(const std::vector<RawMeshletData>& meshletDataCollection);
    std::vector<MeshletGroup> GroupMeshlets(const RawMeshletData& meshletData);
    std::vector<uint32_t> CreateCluster(const MeshletGroup& group, const RawMeshletData& meshletData);
    std::vector<uint32_t> RemapVertices(const std::vector<uint32_t>& indices,
        const std::vector<RawVertex>& vertices,
        const std::unordered_set<uint32_t>& borders,
        std::vector<uint32_t>& remappedIndices,
        std::vector<RawVertex>& remappedVertices,
        std::unordered_set<uint32_t>& remappedBorders);

    bool GenerateNextLOD(const RawMeshletData& meshletData, const std::vector<RawVertex>& vertices, RawMeshletData& lodMeshlets, uint32_t lod, float tLod)
    {
        bool result = false;
        std::vector<MeshletGroup> groups;
        if (meshletData.Meshlets.size() >= 12)
        {
            groups = GroupMeshlets(meshletData);
            result = true;
        }
        else
        {
            groups.push_back({});
            for (uint32_t index = 0; index < meshletData.Meshlets.size(); ++index)
            {
                groups[0].Meshlets.push_back(index);
            }
            result = false;
        }

        std::vector<RawMeshletData> clusterMeshletCollection;

        for (uint32_t groupId = 0; groupId < groups.size(); ++groupId)
        {
            const auto& group = groups[groupId];

            if (group.Meshlets.empty())
            {
                continue;
            }

            const auto& clusterIndices = CreateCluster(group, meshletData);

            if (clusterIndices.size() > 0)
            {
                std::vector<uint32_t> remappedIndices;
                std::vector<RawVertex> remappedVertices;
                std::unordered_set<uint32_t> remappedBoundary;
                auto indexLookup = RemapVertices(clusterIndices, vertices, group.BoundaryVertices, remappedIndices, remappedVertices, remappedBoundary);

                float targetError = std::lerp(0.01f, 0.5f, tLod);
                float threshold = 0.5f;

                float error = 0;
                auto simplifiedIndices = MeshSimplifyFunc(remappedIndices, remappedVertices, group.BoundaryVertices, threshold, targetError, error);

                std::vector<uint32_t> restoredIndices;
                restoredIndices.resize(simplifiedIndices.size());
                for (size_t i = 0; i < simplifiedIndices.size(); ++i)
                {
                    restoredIndices[i] = indexLookup[simplifiedIndices[i]];
                }

                auto clusterMeshlets = GenerateMeshletFunc(vertices, restoredIndices);

                for (auto& meshlet : clusterMeshlets.Meshlets)
                {
                    meshlet.GroupId = groupId;
                    meshlet.Lod = lod;
                }

                clusterMeshletCollection.push_back(clusterMeshlets);
            }
        }

        lodMeshlets = CombineMeshlets(clusterMeshletCollection);

        return result;
    }

    std::vector<uint32_t> RemapVertices(const std::vector<uint32_t>& indices,
        const std::vector<RawVertex>& vertices,
        const std::unordered_set<uint32_t>& boundary,
        std::vector<uint32_t>& remappedIndices,
        std::vector<RawVertex>& remappedVertices,
        std::unordered_set<uint32_t>& remappedBoundary)
    {
        std::unordered_map<uint32_t, size_t> indexMap;
        for (int i = 0; i < indices.size(); ++i)
        {
            uint32_t index = indices[i];
            const auto& iter = indexMap.find(index);
            
            uint32_t newIndex;
            if (iter != indexMap.end())
            {
                newIndex = iter->second;
            }
            else
            {
                size_t vertexPosition = remappedVertices.size();
                indexMap.insert({ index, vertexPosition });
                remappedVertices.push_back(vertices[index]);

                newIndex = vertexPosition;
            }

            remappedIndices.push_back(newIndex);

            if (boundary.find(index) != boundary.end())
            {
                remappedBoundary.insert(newIndex);
            }
        }

        std::vector<uint32_t> indexLookup;
        indexLookup.resize(indexMap.size());
        for (const auto& [oldIndex, newIndex] : indexMap)
        {
            indexLookup[newIndex] = oldIndex;
        }

        return indexLookup;
    }

    RawMeshletData CombineMeshlets(const std::vector<RawMeshletData>& meshletDataCollection)
    {
        RawMeshletData result = {};

        for (const auto& meshletData : meshletDataCollection)
        {
            for (const auto& clusterMeshlet : meshletData.Meshlets)
            {
                Bin3D::Meshlet m(clusterMeshlet);
                m.PrimOffset = result.PrimitiveIndices.size() + clusterMeshlet.PrimOffset;
                m.VertOffset = result.UniqueVertexIB.size() + clusterMeshlet.VertOffset;

                result.Meshlets.push_back(m);
            }

            result.PrimitiveIndices.insert(result.PrimitiveIndices.end(), meshletData.PrimitiveIndices.begin(), meshletData.PrimitiveIndices.end());

            result.UniqueVertexIB.insert(result.UniqueVertexIB.end(), meshletData.UniqueVertexIB.begin(), meshletData.UniqueVertexIB.end());
        }

        return result;
    }

    std::vector<uint32_t> CreateCluster(const MeshletGroup& group, const RawMeshletData& meshletData)
    {
        std::vector<uint32_t> clusterIndices;

        for (uint32_t meshletId : group.Meshlets)
        {
            const auto& meshlet = meshletData.Meshlets[meshletId];

            for (size_t triangleIndex = meshlet.PrimOffset; triangleIndex < meshlet.PrimOffset + meshlet.PrimCount; ++triangleIndex)
            {
                const auto& triangle = meshletData.PrimitiveIndices[triangleIndex];

                clusterIndices.push_back(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i0]);
                clusterIndices.push_back(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i1]);
                clusterIndices.push_back(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i2]);
            }
        }

        return clusterIndices;
    }

    std::vector<MeshletGroup> GroupMeshlets(const RawMeshletData& meshletData)
    {
        std::unordered_map<Edge, MeshletPair> edgeToMeshlets;
        for (uint32_t meshletIndex = 0; meshletIndex < meshletData.Meshlets.size(); ++meshletIndex)
        {
            const auto& meshlet = meshletData.Meshlets[meshletIndex];

            for (int triangleIndex = meshlet.PrimOffset; triangleIndex < meshlet.PrimOffset + meshlet.PrimCount; ++triangleIndex)
            {
                const auto& triangle = meshletData.PrimitiveIndices[triangleIndex];

                auto edges =
                {
                    Edge(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i0], meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i1]),
                    Edge(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i1], meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i2]),
                    Edge(meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i2], meshletData.UniqueVertexIB[meshlet.VertOffset + triangle.i0])
                };

                for (const auto& edge : edges)
                {
                    const auto& iter = edgeToMeshlets.find(edge);
                    if (iter == edgeToMeshlets.end())
                    {
                        MeshletPair pair{ meshletIndex };
                        edgeToMeshlets.insert({ edge, pair });
                    }
                    else
                    {
                        iter->second.AddMeshlet(meshletIndex);
                    }
                }
            }
        }
        std::erase_if(edgeToMeshlets, [](const auto& pair)
        {
                return pair.second.Count == 1;
        });

        std::vector<std::vector<Edge>> meshletToEdges;
        meshletToEdges.resize(meshletData.Meshlets.size());

        for (const auto& [edge, meshletPair] : edgeToMeshlets)
        {
            for (int i = 0; i < meshletPair.Count; ++i)
            {
                meshletToEdges[meshletPair.Meshlets[i]].push_back(edge);
            }
        }


        idx_t vertexCount = meshletData.Meshlets.size(); // vertex count, from the point of view of METIS, where Meshlet = vertex

        // prepare storage for partition data
        // each vertex will get its partition index inside this vector after the edge-cut
        std::vector<idx_t> partition;
        partition.resize(vertexCount);

        // xadj
        std::vector<idx_t> xadjacency;
        xadjacency.reserve(vertexCount + 1);

        // adjncy
        std::vector<idx_t> edgeAdjacency;
        // weight of each edge
        std::vector<idx_t> edgeWeights;


        for (uint32_t meshletIndex = 0; meshletIndex < meshletData.Meshlets.size(); ++meshletIndex)
        {
            const auto& edges = meshletToEdges[meshletIndex];
            size_t edgeAdjacencyStart = edgeAdjacency.size();

            for (const auto& edge : edges)
            {
                const auto& meshletsIter = edgeToMeshlets.find(edge);
                const auto& meshletPair = meshletsIter->second;

                auto neighbourMeshlet = meshletPair.GetConnectedMeshlet(meshletIndex);

                const auto& edgeAdjacencyIter = std::find(edgeAdjacency.begin() + edgeAdjacencyStart, edgeAdjacency.end(), neighbourMeshlet);
                if (edgeAdjacencyIter == edgeAdjacency.end())
                {
                    edgeAdjacency.push_back(neighbourMeshlet);
                    edgeWeights.push_back(1);
                }
                else
                {
                    auto diff = std::distance(edgeAdjacency.begin(), edgeAdjacencyIter);
                    edgeWeights[diff]++;
                }
            }

            xadjacency.push_back(edgeAdjacencyStart);

        }

        xadjacency.push_back(edgeAdjacency.size());

        https://jglrxavpok.github.io/2024/01/19/recreating-nanite-lod-generation.html
        
        idx_t ncon = 1; // only one constraint, minimum required by METIS
        idx_t nparts = vertexCount / 4; // groups of 4
        idx_t options[METIS_NOPTIONS];
        METIS_SetDefaultOptions(options);

        // edge-cut, ie minimum cost between groups.
        options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
        options[METIS_OPTION_CCORDER] = 1; // identify connected components first

        idx_t edgeCut; // final cost of the cut found by METIS
        int result = METIS_PartGraphKway(
            &vertexCount,
            &ncon,
            xadjacency.data(),
            edgeAdjacency.data(),
            nullptr, /* vertex weights */
            nullptr, /* vertex size */
            edgeWeights.data(),
            &nparts,
            nullptr,
            nullptr,
            options,
            &edgeCut,
            partition.data()
        );

        if (result != METIS_OK)
        {
            spdlog::error("METIS partitioning failed: {}", result);
        }

        std::vector<MeshletGroup> groups;
        groups.resize(nparts);

        for (size_t meshletIndex = 0; meshletIndex < meshletData.Meshlets.size(); ++meshletIndex)
        {
            auto group = partition[meshletIndex];
            groups[group].Meshlets.push_back(meshletIndex);
        }

        for (auto& group : groups)
        {
            for (uint32_t meshletIndex : group.Meshlets)
            {
                for (const auto& edge : meshletToEdges[meshletIndex])
                {
                    auto& meshletPair = edgeToMeshlets.find(edge)->second;

                    if (std::find(group.Meshlets.begin(), group.Meshlets.end(), meshletPair.GetConnectedMeshlet(meshletIndex)) == group.Meshlets.end())
                    {
                        group.BoundaryVertices.insert(edge.A);
                        group.BoundaryVertices.insert(edge.B);
                    }
                }
            }
        }
        return groups;
    }

    RawMeshletData MeshProcessor::ProcessMesh(const RawMesh& mesh)
    {
        auto meshlets = GenerateMeshletFunc(mesh.Vertices, mesh.Indices);
        uint32_t maxLod = std::ceil(std::log2(meshlets.Meshlets.size()));

        spdlog::info("Expected max LOD: {}", maxLod - 1);
        spdlog::info("LOD{}: Meshlets count: {}; Faces count: {}; 100% : 100%", 0, meshlets.Meshlets.size(), meshlets.PrimitiveIndices.size());
        size_t initialFacesCount = meshlets.PrimitiveIndices.size();

        std::vector<RawMeshletData> lodMeshletData;
        lodMeshletData.reserve(maxLod);
        lodMeshletData.push_back(meshlets);

        for (int lod = 1; lod < maxLod; ++lod)
        {
            float tLod = lod / ((float)maxLod - 1);

            RawMeshletData lodMeshlet;
            if (GenerateNextLOD(lodMeshletData[lod - 1], mesh.Vertices, lodMeshlet, lod, tLod))
            {
                lodMeshletData.push_back(std::move(lodMeshlet));
            }
            else
            {
                lodMeshletData.push_back(std::move(lodMeshlet));
                break;
            }
            const auto& lodMeshlets = lodMeshletData[lod];
            size_t facesCount = lodMeshlets.PrimitiveIndices.size();

            int totalPercent = ((float)facesCount / initialFacesCount) * 100;
            int prevPercent = ((float)facesCount / lodMeshletData[lod - 1].PrimitiveIndices.size()) * 100;

            spdlog::info("LOD{}: Meshlets count: {}; Faces count: {}: {}% : {}%", 
                lod, 
                lodMeshlets.Meshlets.size(), 
                lodMeshlets.PrimitiveIndices.size(), 
                prevPercent, 
                totalPercent);
        }

        return CombineMeshlets(lodMeshletData);
    }

}