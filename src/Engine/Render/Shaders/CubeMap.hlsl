#include "ShaderTypes.h"

cbuffer CubeTexture : register(b0)
{
    int CubeTextureIndex;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

#include "BaseLayout.hlsl"

static const float3 Vertices[8] =
{
    // Fill in the front face vertex data.
    float3(-1.0, -1.0, -1.0),
    float3(-1.0, +1.0, -1.0),
    float3(+1.0, +1.0, -1.0),
    float3(+1.0, -1.0, -1.0),

    // Fill in the back face vertex data.
    float3(-1.0, -1.0, +1.0),
    float3(+1.0, -1.0, +1.0),
    float3(+1.0, +1.0, +1.0),
    float3(-1.0, +1.0, +1.0)
};

static const uint Indices[36] =
{
    // Fill in the front face index data
    0, 1, 2,
    0, 2, 3,

    // Fill in the back face index data
    4, 5, 6,
    4, 6, 7,

    // Fill in the top face index data
    1, 7, 6,
    1, 6, 2,

    // Fill in the bottom face index data
    0, 3, 5,
    0, 5, 4,

    // Fill in the left face index data
    4, 7, 1,
    4, 1, 0,

    // Fill in the right face index data
    3, 2, 6,
    3, 6, 5
};

struct VertexShaderOutput
{
    float3 TextureCoord : TEXCOORD;
    float4 PositionH : SV_Position;
};

struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
};

float3 SRGBToLinear(float3 sRGBCol)
{
    float3 linearRGBLo = sRGBCol / 12.92;
    float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, 2.4);
    float3 linearRGB = (sRGBCol <= 0.04045) ? linearRGBLo : linearRGBHi;
    return linearRGB;
}

VertexShaderOutput mainVS(uint indexId : SV_VertexID)
{
    VertexShaderOutput OUT;
    float3 positionL = Vertices[Indices[indexId]];
    float3 posW = positionL + FrameCB.EyePos;
    OUT.PositionH = mul(float4(posW, 1.0f), FrameCB.ViewProj).xyww;
    OUT.TextureCoord = positionL;

    return OUT;
}

PixelShaderOutput mainPS(VertexShaderOutput IN)
{
    PixelShaderOutput OUT;

    TextureCube baseColorTexture = ResourceDescriptorHeap[CubeTextureIndex];
    float4 baseColor = baseColorTexture.Sample(gsamPointWrap, IN.TextureCoord);
    baseColor = float4(SRGBToLinear(baseColor.rgb), baseColor.a);
    OUT.Color = baseColor;

    return OUT;
}