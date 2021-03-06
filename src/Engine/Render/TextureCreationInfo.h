#pragma once

#include <DirectXHashes.h>
#include <d3d12.h>

#include <compare>

namespace Engine::Render
{
    struct TextureCreationInfo
    {
        D3D12_RESOURCE_DESC description;
        D3D12_CLEAR_VALUE clearValue;

        auto operator<=>(const TextureCreationInfo& other) const = default;
    };
} // namespace Engine::Render

namespace std
{
    template <>
    struct hash<Engine::Render::TextureCreationInfo>
    {
        size_t operator()(const Engine::Render::TextureCreationInfo &key) const
        {
            return std::hash_combine(
                key.description,
                key.clearValue);
        }
    };
} // namespace std
