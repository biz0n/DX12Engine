#include <spdlog/spdlog.h>

#include <Importer/SceneImporter.h>
#include <Writer/ImageProcessor.h>
#include <Writer/BinaryWriter.h>

#include <Bin3D/Reader/BinaryReader.h>

#include <assert.hpp>

int main(int argc, char* argv[])
{
   // auto path = R"(C:\Users\Maxim\Downloads\Main\Main\NewSponza_Main_Blender_glTF - Copy.gltf)";
    auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\gltf2\sponza\sponza.gltf)";
   // auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\DamagedHelmet\glTF\DamagedHelmet.gltf)";
   // auto path = R"(C:\Users\Maxim\Documents\dev\3dmodels\cube\plane.gltf)";
  //  auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\FlightHelmet\glTF\FlightHelmet.gltf)";
  //  auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\Corset\glTF\Corset.gltf)";
  //  auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\Avocado\glTF\Avocado.gltf)";
 //   auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\BoomBoxWithAxes\glTF\BoomBoxWithAxes.gltf)";
  //  auto path = R"(C:\Users\Maxim\Documents\dev\3d\src\Engine\Resources\Scenes\glTF-Sample-Models-master\2.0\BoomBox\glTF\BoomBox.gltf)";

    std::filesystem::path output = R"(C:\Users\Maxim\Documents\dev\3d\3DModels\sponza)";
    
#if 1
    SceneConverter::Importer::SceneImporter importer;

    auto scene = importer.LoadScene(path);

    SceneConverter::Writer::BinaryWriter writer;
    SceneConverter::Writer::ImageProcessor imageProcessor;
    writer.Prepare(output);

    imageProcessor.ProcessImages(output, scene);

    writer.WriteScene(output, "model.bin3d", scene);
#endif
    Bin3D::Reader::BinaryReader reader;

    auto loadedScene = reader.ReadScene(output / "model.bin3d");

    spdlog::info("Done!");

    return 0;
}