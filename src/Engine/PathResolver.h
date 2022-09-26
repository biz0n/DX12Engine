#pragma once

#include <filesystem>

namespace Engine
{
    class PathResolver
    {
    public:
        PathResolver() = delete;

        static std::filesystem::path GetResourcePath(const std::string& path);
        static std::filesystem::path GetResourcePath(const char* path);

        static std::filesystem::path GetShaderPath(const std::string& path);
        static std::filesystem::path GetShaderPath(const char* path);
    };
}
