#pragma once

#include <Types.h>
#include <Hash.h>
#include <HAL/DirectXHashes.h>
#include <d3d12.h>
#include <vector>

#include <compare>
#include <tuple>

namespace Engine::HAL
{
    struct ShaderCreationInfo
    {
        std::string path;
        std::string entryPoint;
        std::string target;
        std::vector<std::string> defines;

        ShaderCreationInfo(const std::string& path, const std::string& entryPoint, const std::string& target) : path(path), entryPoint(entryPoint), target(target)
        {

        }

        auto operator<=>(const ShaderCreationInfo &other) const
        {
            auto left = std::tie(this->path, this->entryPoint, this->target, this->defines);
            auto right = std::tie(other.path, other.entryPoint, other.target, other.defines);

            if (left < right)
            {
                return std::strong_ordering::less;
            }
            else if (left > right)
            {
                return std::strong_ordering::greater;
            }

            return std::strong_ordering::equal;
        }
    };
} // namespace Engine::Render

namespace std
{

    template <>
    struct hash<Engine::HAL::ShaderCreationInfo>
    {
        size_t operator()(const Engine::HAL::ShaderCreationInfo &key) const
        {
            return std::hash_combine(
                std::hash<std::string>{}(key.path),
                std::hash<std::string>{}(key.entryPoint),
                std::hash<std::string>{}(key.target),
                std::hash_combine(key.defines.begin(), key.defines.end()));
        }
    };

} // namespace std