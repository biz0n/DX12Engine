#include "ShaderTypes.h"
#include "Vertex.hlsl"

cbuffer Mesh : register(b0)
{
    int MeshIndex;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

StructuredBuffer<MeshUniform> Meshes : register(t0, space1);

struct VertexOut
{
    float4 PositionH : SV_Position;
    float3 PositionW : POSITION0;
  //  float4 ShadowPosH : POSITION1;
    float3 NormalW : NORMAL;
  //  float2 TextureCoord : TEXCOORD;
  //  float3x3 TBN : TBN;
    
    uint indexId : INDEX;
};

struct VertexOut_
{
    float4 PositionHS : SV_Position;
    float3 PositionVS : POSITION0;
    float3 Normal : NORMAL0;
    uint MeshletIndex : COLOR0;
};

uint3 UnpackPrimitive(uint primitive)
{
    // Unpacks a 10 bits per index triangle from a 32-bit uint.
    return uint3(primitive & 0x3FF, (primitive >> 10) & 0x3FF, (primitive >> 20) & 0x3FF);
}

uint3 GetPrimitive(Meshlet m, uint index, StructuredBuffer<uint> primitiveIndices)
{
    
    return UnpackPrimitive(primitiveIndices[(m.PrimOffset + index)]);
}

uint GetVertexIndex(Meshlet m, uint localIndex, ByteAddressBuffer uniqueVertexIndices)
{
    return uniqueVertexIndices.Load((m.VertOffset + localIndex) * 4);
}

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex, MeshUniform meshInfo, 
                               StructuredBuffer<Vertex1P> verticesCoordinates, StructuredBuffer<Vertex1N1UV1T> verticesProperties, uint gtid)
{
    
    Vertex1P v = verticesCoordinates[(vertexIndex)];
    Vertex1N1UV1T vp = verticesProperties[(vertexIndex)];

    VertexOut vout;
    
    float4 posW = mul(float4(v.PositionL, 1.0f), meshInfo.World);
    float3 normalW = mul(vp.NormalL, (float3x3) meshInfo.InverseTranspose);

    float3 T = normalize(mul(vp.Tangent.xyz, (float3x3) meshInfo.World));
    float3 N = normalize(mul(vp.NormalL, (float3x3) meshInfo.World));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * vp.Tangent.w;

    float3x3 TBN = float3x3(T, B, N);

   // VertexOut vout;
    //vout.PositionW = mul(float4(v.PositionL, 1.0f), FrameCB.View).xyz;
    //vout.PositionH = mul(float4(v.PositionL, 1.0f), FrameCB.ViewProj);
   // vout.Normal = vp.NormalL;
   // vout.MeshletIndex = meshletIndex;
    
    
    vout.NormalW = normalW;
    vout.PositionW = posW.xyz;
    //OUT.ShadowPosH = mul(posW, FrameCB.ShadowTransform);
    vout.PositionH = mul(posW, FrameCB.ViewProj);
   // vout.PositionW = v.PositionL.xyz;
  //  vout.ShadowPosH = mul(posW, FrameCB.ShadowTransform);
   // vout.PositionH = float4(v.PositionL, 1);
  //  vout.TBN = TBN;
  //  vout.TextureCoord = vp.TextureCoord;
    vout.indexId = meshletIndex;


    return vout;
}



[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mainMS(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out indices uint3 tris[126],
    out vertices VertexOut verts[64]
)
{
    MeshUniform meshInfo = Meshes[(MeshIndex)];
    
    StructuredBuffer<Vertex1P> verticesCoordinates = ResourceDescriptorHeap[(meshInfo.VertexBufferIndex)];
    StructuredBuffer<Vertex1N1UV1T> verticesProperties = ResourceDescriptorHeap[(meshInfo.VertexPropertiesBufferIndex)];
    ByteAddressBuffer uniqueVertexIndices = ResourceDescriptorHeap[(meshInfo.UniqueVertexIndexBufferIndex)];
    StructuredBuffer<uint> primitiveIndices = ResourceDescriptorHeap[(meshInfo.PrimitiveIndexBufferIndex)];

    
    StructuredBuffer<Meshlet> meshlets = ResourceDescriptorHeap[(meshInfo.MeshletBufferIndex)];
 
    Meshlet m = meshlets[(gid)];

    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        tris[gtid] = GetPrimitive(m, gtid, primitiveIndices);
    }

    if (gtid < m.VertCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid, uniqueVertexIndices);
        verts[gtid] = GetVertexAttributes(gid, vertexIndex, meshInfo, verticesCoordinates, verticesProperties, gtid);
    }
}
