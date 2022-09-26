#include "BinaryReader.h"

#include <Bin3D/Reader/BinaryHeader.h>

#include <iostream>
#include <fstream>
#include <vector>

namespace Bin3D::Reader
{
    std::shared_ptr<Scene> BinaryReader::ReadScene(const std::filesystem::path& path)
    {
        std::ifstream fs(path.c_str(), std::ios_base::in | std::ios_base::binary);

        fs.unsetf(std::ios::skipws);

        fs.seekg(0, std::ios::end);
        auto size = fs.tellg();
        fs.seekg(0, std::ios::beg);

        std::vector<char> vec;
        vec.resize(size);

        fs.read(&vec[0], size);

        fs.close();

        auto scene = std::make_shared<Scene>( path.parent_path(), std::move(vec) );

        return scene;
    }
}
