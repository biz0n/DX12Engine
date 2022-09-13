#pragma once

#include <cstdint>

namespace Engine::Scene::Reader
{
    struct HeaderField
    {
        uint32_t Offset;
        uint32_t Size;
    };

	struct BinaryHeader
	{
        HeaderField NodesInfo;
        HeaderField MeshesInfo;
        HeaderField MaterialsInfo;
        HeaderField LightsInfo;
        HeaderField CamerasInfo;
        HeaderField SamplersInfo;

        HeaderField MeshIndicesStorageInfo;
        HeaderField VerticesStorageInfo;
        HeaderField IndicesStorageInfo;
        HeaderField ImagePathsInfo;
        HeaderField StringsStorageInfo;
	};
}
