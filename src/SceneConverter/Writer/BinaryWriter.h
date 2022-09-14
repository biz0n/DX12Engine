#pragma once

#include <Model/Scene.h>

#include <Scene/Reader/BinaryHeader.h>

#include <filesystem>
#include <vector>
#include <fstream>

namespace SceneConverter::Writer
{
    class BinaryWriter
    {
    public:
        void Prepare(const std::filesystem::path& path);
        void WriteScene(const std::filesystem::path& path, const std::string& filename, const Model::Scene& scene);
    private:

        template <typename T>
        Engine::Scene::DataRegion Write(std::ofstream& stream, const std::vector<T>& data)
        {
            Engine::Scene::DataRegion field = {};
            field.Offset = stream.tellp();
            field.Size = data.size();

            stream.write(reinterpret_cast<const char*>(data.data()), sizeof(T) * data.size());
            return field;
        }
    };
}