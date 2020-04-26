#ifndef SHADERTYPES_H
#define SHADERTYPES_H

#ifdef __cplusplus

#include <DirectXMath.h>

using float2 = DirectX::XMFLOAT2;
using float3 = DirectX::XMFLOAT3;
using float4 = DirectX::XMFLOAT4;

using float3x3 = DirectX::XMFLOAT3X3;
using float4x4 = DirectX::XMFLOAT4X4;

#endif

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
};

struct LightUniform
{
    float4 PositionWS;
    float4 DirectionWS;
    float4 Color;

    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;

    int LightType;
    bool Enabled;
    float2 Padding;
};

struct LightProperties
{
    float3 GlobalAmbient;
    int LightsCount;
    LightUniform Lights[MAX_LIGHTS];
};

struct FrameUniform
{
    float4x4 ViewProj;
    float3 EyePos;
    float Padding;

    LightProperties LightProperties;
};

#endif