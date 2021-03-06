#include "ShaderTypes.h"
#include "Vertex.hlsl"

ConstantBuffer<MeshUniform> ObjectCB : register(b0);

ConstantBuffer<FrameUniform> FrameCB : register(b1);

ConstantBuffer<MaterialUniform> MaterialCB : register(b2);

Texture2D baseColorTexture : register(t0);

SamplerState gsamPointWrap : register(s0);

struct VertexShaderOutput
{
    float2 TextureCoord : TEXCOORD;
    float4 PositionH : SV_Position;
};

VertexShaderOutput mainVS(Vertex1P1N1UV1T IN)
{
    VertexShaderOutput OUT;

    OUT.TextureCoord = IN.TextureCoord;

    float4 posW = mul(float4(IN.PositionL, 1.0f), ObjectCB.World);
    OUT.PositionH = mul(posW, FrameCB.ViewProj);

    return OUT;
}

void mainPS(VertexShaderOutput IN)
{
    float4 baseColor = MaterialCB.BaseColor;
    if (MaterialCB.HasBaseColorTexture)
    {
        baseColor = baseColorTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }

    clip(baseColor.a - MaterialCB.Cutoff);
}