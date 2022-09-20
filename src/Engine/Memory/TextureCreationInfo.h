#pragma once

#include <Types.h>
#include <HAL/DirectXHashes.h>
#include <d3d12.h>

#include <compare>

namespace Engine::Memory
{
    struct TextureCreationInfo
    {
        D3D12_RESOURCE_DESC description;
        std::optional<D3D12_CLEAR_VALUE> clearValue;

        auto operator<=>(const TextureCreationInfo& other) const = default;
    };
} // namespace Engine::Render

namespace std
{
    template <>
    struct hash<Engine::Memory::TextureCreationInfo>
    {
        size_t operator()(const Engine::Memory::TextureCreationInfo &key) const
        {
            return std::hash_combine(
                key.description,
                key.clearValue);
        }
    };
} // namespace std
