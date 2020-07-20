#ifndef SHADERTYPES_H
#define SHADERTYPES_H

#ifdef __cplusplus

#include <DirectXMath.h>

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

using float3x3 = DirectX::XMFLOAT3X3;
using float4x4 = DirectX::XMFLOAT4X4;

#define CB_ALIGN(x) alignas(x) 

#else
#define CB_ALIGN(x)
#endif

#define aligned_bool CB_ALIGN(4) bool

#define MAX_LIGHTS 16

// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct MeshUniform
{
    float4x4 World;
    float4x4 InverseTranspose;
};

struct MaterialUniform
{
    float4 Emissive;
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float MetallicFactor;
    float RoughnessFactor;
    float SpecularPower;
    float NormalScale;

    aligned_bool HasBaseColorTexture;
    aligned_bool HasNormalTexture;
    aligned_bool HasMetallicRoughnessTexture;
    aligned_bool HasOcclusionTexture;

    aligned_bool HasEmissiveTexture;

    float3 Padding;
};

struct LightUniform
{
    float3 PositionWS;
    int LightType;

    float3 DirectionWS;
    aligned_bool Enabled;

    float3 Color;
    float ConstantAttenuation;

    float LinearAttenuation;
    float QuadraticAttenuation;
    float InnerConeAngle;
    float OuterConeAngle;
};

struct FrameUniform
{
    float4x4 ViewProj;
    float3 EyePos;
    int LightsCount;
};

#endif