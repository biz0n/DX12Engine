#include "ShaderTypes.h"

cbuffer CubeTexture : register(b0)
{
    int CubeTextureIndex;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

#include "BaseLayout.hlsl"

struct VertexPosColor
{
    float3 PositionL : POSITION;
    float3 NormalL : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float4 Tangent : TANGENT;
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

VertexShaderOutput mainVS(VertexPosColor IN)
{
    VertexShaderOutput OUT;
 
    float3 posW = IN.PositionL + FrameCB.EyePos;
    OUT.PositionH = mul(float4(posW, 1.0f), FrameCB.ViewProj).xyww;
    OUT.TextureCoord = IN.PositionL;

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