#include "BinaryWriter.h"


#include <iostream>

using namespace Engine::Scene::Reader;

namespace SceneConverter::Writer
{
    void BinaryWriter::Prepare(const std::filesystem::path& path)
    {
        std::filesystem::remove_all(path);
        std::filesystem::create_directories(path);
    }

	void BinaryWriter::WriteScene(const std::filesystem::path& path, const std::string& filename, const Model::Scene& scene)
	{
        const auto filePath = path / filename;

        std::ofstream fs(filePath.c_str(), std::ios::out | std::ios_base::binary);

        fs.seekp(sizeof(BinaryHeader));

        BinaryHeader header = {};
        header.NodesInfo = Write(fs, scene.GetNodes());
        header.MeshesInfo = Write(fs, scene.GetMeshes());
        header.MaterialsInfo = Write(fs, scene.GetMaterials());
        header.LightsInfo = Write(fs, scene.GetLights());
        header.CamerasInfo = Write(fs, scene.GetCameras());
        header.SamplersInfo = Write(fs, scene.GetSamplers());
        header.MeshIndicesStorageInfo = Write(fs, scene.GetNodeMeshIndicesStorage());
        header.VerticesStorageInfo = Write(fs, scene.GetVerticesStorage());
        header.IndicesStorageInfo = Write(fs, scene.GetIndicesStorage());
        header.ImagePathsInfo = Write(fs, scene.GetImagePaths());
        header.StringsStorageInfo = Write(fs, scene.GetStringsStorage());

        fs.seekp(0);
        fs.write(reinterpret_cast<const char*>(&header), sizeof(BinaryHeader));
        
        fs.close();
	}
}
