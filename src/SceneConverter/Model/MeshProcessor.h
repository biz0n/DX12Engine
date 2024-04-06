#pragma once

#include <Model/RawMesh.h>

#include <span>

namespace SceneConverter::Model
{
    class MeshProcessor
    {
    public:
        RawMeshletData ProcessMesh(const RawMesh& mesh);
    };
}
