#pragma once

#include <Scene/DataRegion.h>

namespace Engine::Scene::Reader
{
    struct BinaryHeader
    {
        DataRegion Nodes;
        DataRegion Meshes;
        DataRegion Materials;
        DataRegion Lights;
        DataRegion Cameras;
        DataRegion Samplers;

        DataRegion NodeMeshIndicesStorage;
        DataRegion VerticesStorage;
        DataRegion IndicesStorage;
        DataRegion ImagePaths;
        DataRegion StringsStorage;
    };
}
