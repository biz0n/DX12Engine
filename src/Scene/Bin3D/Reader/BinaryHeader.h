#pragma once

#include <Bin3D/DataRegion.h>

namespace Bin3D::Reader
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
