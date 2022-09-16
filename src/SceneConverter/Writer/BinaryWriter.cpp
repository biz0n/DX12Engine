#include "BinaryWriter.h"

#include <iostream>

using namespace Bin3D::Reader;

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
        header.Nodes = Write(fs, scene.GetNodes());
        header.Meshes = Write(fs, scene.GetMeshes());
        header.Materials = Write(fs, scene.GetMaterials());
        header.Lights = Write(fs, scene.GetLights());
        header.Cameras = Write(fs, scene.GetCameras());
        header.Samplers = Write(fs, scene.GetSamplers());
        header.VerticesStorage = Write(fs, scene.GetVerticesStorage());
        header.IndicesStorage = Write(fs, scene.GetIndicesStorage());
        header.ImagePaths = Write(fs, scene.GetImagePaths());
        header.StringsStorage = Write(fs, scene.GetStringsStorage());

        fs.seekp(0);
        fs.write(reinterpret_cast<const char*>(&header), sizeof(BinaryHeader));
        
        fs.close();
    }
}
