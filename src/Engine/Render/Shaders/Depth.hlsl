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

uint GetVertexIndex(uint localIndex, ByteAddressBuffer indices, int indexSize)
{
    if (indexSize == 4) // 32-bit Vertex Indices
    {
        return indices.Load(localIndex * 4);
    }
    else // 16-bit Vertex Indices
    {
        // Byte address must be 4-byte aligned.
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
        uint indexPair = indices.Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

VertexShaderOutput mainVS(uint indexId : SV_VertexID)
{
    MeshUniform ObjectCB = Meshes[MeshIndex];
    StructuredBuffer<Vertex1P> verticesCoordinates = ResourceDescriptorHeap[ObjectCB.VertexBufferIndex];
    StructuredBuffer<Vertex1N1UV1T> verticesProperties = ResourceDescriptorHeap[ObjectCB.VertexPropertiesBufferIndex];
    ByteAddressBuffer indices = ResourceDescriptorHeap[ObjectCB.IndexBufferIndex];

    uint vertexId = GetVertexIndex(indexId, indices, ObjectCB.IndexSize);

    Vertex1P IN = verticesCoordinates[vertexId];
    Vertex1N1UV1T properties = verticesProperties[vertexId];

    VertexShaderOutput OUT;

    OUT.TextureCoord = properties.TextureCoord;

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