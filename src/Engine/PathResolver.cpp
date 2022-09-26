#include "PathResolver.h"

namespace Engine
{
    static std::filesystem::path mResourcesPath = std::filesystem::absolute(RESOURCES_RELATIVE_PATH);

    std::filesystem::path PathResolver::GetResourcePath(const std::string& path)
    {
        return GetResourcePath(path.c_str());
    }

    std::filesystem::path PathResolver::GetResourcePath(const char* path)
    {
        return mResourcesPath / path;
    }

    std::filesystem::path PathResolver::GetShaderPath(const std::string& path)
    {
        return GetShaderPath(path.c_str());
    }

    std::filesystem::path PathResolver::GetShaderPath(const char* path)
    {
        return path;
    }
}