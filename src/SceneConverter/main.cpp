#include <iostream>

#include <Importer/SceneImporter.h>
#include <Writer/ImageProcessor.h>
#include <Writer/BinaryWriter.h>

#include <Bin3D/Reader/BinaryReader.h>

int main(int argc, char* argv[])
{
    auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\gltf2\sponza\sponza.gltf)";

    std::filesystem::path output = R"(C:\Users\Maxim\Documents\dev\3d\3DModels\sponza)";
    //auto path = R"(C:\Users\Maxim\Downloads\Main\Main\NewSponza_Main_Blender_glTF - Copy.gltf)";

    SceneConverter::Importer::SceneImporter importer;

    auto scene = importer.LoadScene(path);

    SceneConverter::Writer::BinaryWriter writer;
    SceneConverter::Writer::ImageProcessor imageProcessor;
    writer.Prepare(output);

    imageProcessor.ProcessImages(output, scene);

    writer.WriteScene(output, "sponza.bin3d", scene);

    Bin3D::Reader::BinaryReader reader;

    std::cout << "Start" << std::endl;

    auto loadedScene = reader.ReadScene(output / "sponza.bin3d");

    std::cout << "Done" << std::endl;

    return 0;
}