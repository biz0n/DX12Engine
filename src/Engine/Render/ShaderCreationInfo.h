#pragma once

#include <Types.h>
#include <Hash.h>
#include <Render/DirectXHashes.h>
#include <d3d12.h>
#include <vector>

#include <compare>
#include <tuple>

namespace Engine::Render
{
    struct ShaderCreationInfo
    {
        String path;
        String entryPoint;
        String target;
        std::vector<D3D_SHADER_MACRO> defines;

        ShaderCreationInfo(String path, String entryPoint, String target) : path(path), entryPoint(entryPoint), target(target)
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
    struct hash<Engine::Render::ShaderCreationInfo>
    {
        size_t operator()(const Engine::Render::ShaderCreationInfo &key) const
        {
            return std::hash_combine(
                std::hash<std::string>{}(key.path),
                std::hash<std::string>{}(key.entryPoint),
                std::hash<std::string>{}(key.target),
                std::hash_combine(key.defines.begin(), key.defines.end()));
        }
    };

} // namespace std