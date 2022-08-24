#include "ShaderTypes.h"
#include "Vertex.hlsl"

cbuffer Mesh : register(b0)
{
    int MeshIndex;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

StructuredBuffer<MeshUniform> Meshes : register(t0, space1);

StructuredBuffer<MaterialUniform> Materials : register(t1, space1);

#include "BaseLayout.hlsl"

struct VertexShaderOutput
{
    float2 TextureCoord : TEXCOORD;
    float4 PositionH : SV_Position;
};

VertexShaderOutput mainVS(uint indexId : SV_VertexID)
{
    MeshUniform ObjectCB = Meshes[MeshIndex];
    StructuredBuffer<Vertex1P1N1UV1T> vertices = ResourceDescriptorHeap[ObjectCB.VertexBufferIndex];
    StructuredBuffer<uint> indices = ResourceDescriptorHeap[ObjectCB.IndexBufferIndex];

    uint vertexId = indices[indexId];

    Vertex1P1N1UV1T IN = vertices[vertexId];

    VertexShaderOutput OUT;

    OUT.TextureCoord = IN.TextureCoord;

    float4 posW = mul(float4(IN.PositionL, 1.0f), ObjectCB.World);
    OUT.PositionH = mul(posW, FrameCB.ViewProj);

    return OUT;
}

void mainPS(VertexShaderOutput IN)
{
    MaterialUniform MaterialCB = Materials[Meshes[MeshIndex].MaterialIndex];
    
    float4 baseColor = MaterialCB.BaseColor;
    if (MaterialCB.HasBaseColorTexture)
    {
        Texture2D<float4> baseColorTexture = ResourceDescriptorHeap[MaterialCB.BaseColorIndex];
        baseColor = baseColorTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }

    clip(baseColor.a - MaterialCB.Cutoff);
}