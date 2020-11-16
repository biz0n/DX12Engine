#include "ShaderTypes.h"

ConstantBuffer<FrameUniform> FrameCB : register(b0);

TextureCube cubeTexture : register(t0);

SamplerState gsamPointWrap : register(s0);

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

VertexShaderOutput mainVS(VertexPosColor IN)
{
    VertexShaderOutput OUT;
 
    OUT.PositionH = mul(IN.PositionL, FrameCB.ViewProj);
    OUT.TextureCoord = IN.PositionL;

    return OUT;
}

PixelShaderOutput mainPS(VertexShaderOutput IN)
{
    PixelShaderOutput OUT;

    OUT.Color = cubeTexture.Sample(gsamPointWrap, IN.TextureCoord);

    return OUT;
}