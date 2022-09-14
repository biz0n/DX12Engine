#pragma once

#include <DirectXMath.h>

namespace Bin3D
{
    enum class AlphaMode : uint8_t
    {
        Opaque,
        Mask,
        Blend,
    };

    struct TextureInfo
    {
        uint8_t texCoord = 0;
    };

    struct NormalTextureInfo : TextureInfo
    {
        float scale = 1;
    };

    struct EmmissiveTextureInfo : TextureInfo
    {
        float strength = 1;
    };

    struct BaseColor
    {
        TextureInfo info;
        DirectX::XMFLOAT4 baseColor = { 1, 1, 1, 1 };
    };

    struct MetallicRaughness
    {
        TextureInfo info;
        float metallicFactor = 1;
        float roughnessFactor = 1;
    };

    struct Emissive
    {
        EmmissiveTextureInfo info;
        DirectX::XMFLOAT3 factor = { 0, 0, 0 };
    };

    struct MaterialProperties
    {
        BaseColor baseColor;
        MetallicRaughness metallicRaughness;
        NormalTextureInfo normalTextureInfo;
        Emissive emissive;
        AlphaMode alphaMode = AlphaMode::Opaque;
        float alphaCutoff = 0.5f;
        bool doubleSided = false;
        bool unlit = false;
    };

    struct Material
    {
        MaterialProperties MaterialProperties;

        uint16_t BaseColorTextureIndex;
        uint16_t NormalTextureIndex;
        uint16_t MetallicRoughnessTextureIndex;
        uint16_t AmbientOcclusionTextureIndex;
        uint16_t EmissiveTextureIndex;

        uint16_t BaseColorTextureSamplerIndex;
        uint16_t NormalTextureSamplerIndex;
        uint16_t MetallicRoughnessTextureSamplerIndex;
        uint16_t AmbientOcclusionTextureSamplerIndex;
        uint16_t EmissiveTextureSamplerIndex;
    };

} // namespace Engine::Scene