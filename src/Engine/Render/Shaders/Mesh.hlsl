#include "ShaderTypes.h"
#include "Vertex.hlsl"

cbuffer Mesh : register(b0)
{
    int MeshIndex;
};

cbuffer LodIndex : register(b1)
{
    int Lod;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

StructuredBuffer<MeshUniform> Meshes : register(t0, space1);

struct VertexOut
{
    float4 PositionH : SV_Position;
    float3 PositionW : POSITION0;
  //  float4 ShadowPosH : POSITION1;
    float3 NormalW : NORMAL;
    
    float2 TextureCoord : TEXCOORD;
  //  float3x3 TBN : TBN;
    uint indexId : INDEX;
    uint lod : INDEX1;
    uint group : INDEX2;
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

uint GetVertexIndex(Meshlet m, uint localIndex, ByteAddressBuffer uniqueVertexIndices, int indexSize)
{
    localIndex = m.VertOffset + localIndex;

    if (indexSize == 4) // 32-bit Vertex Indices
    {
        return uniqueVertexIndices.Load(localIndex * 4);
    }
    else // 16-bit Vertex Indices
    {
        // Byte address must be 4-byte aligned.
        uint wordOffset = (localIndex & 0x1);
        uint byteOffset = (localIndex / 2) * 4;

        // Grab the pair of 16-bit indices, shift & mask off proper 16-bits.
        uint indexPair = uniqueVertexIndices.Load(byteOffset);
        uint index = (indexPair >> (wordOffset * 16)) & 0xffff;

        return index;
    }
}

VertexOut GetVertexAttributes(uint meshletIndex, uint vertexIndex, MeshUniform meshInfo, 
                               StructuredBuffer<Vertex1P> verticesCoordinates, StructuredBuffer<Vertex1N1UV1T> verticesProperties, uint gtid)
{
    
    Vertex1P v = verticesCoordinates[(vertexIndex)];
    Vertex1N1UV1T vp = verticesProperties[(vertexIndex)];

    VertexOut vout;
    
    float4 posW = mul(float4(v.PositionL, 1.0f), meshInfo.World);
    float3 n = vp.NormalL;
    float3 normalW = mul(n, (float3x3) meshInfo.InverseTranspose);

    float3 T = normalize(mul(vp.Tangent.xyz, (float3x3) meshInfo.World));
    float3 N = normalize(mul(n, (float3x3) meshInfo.World));
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
    vout.TextureCoord = vp.TextureCoord;
    vout.indexId = meshletIndex;


    return vout;
}

struct Payload
{
    uint meshletIndex;
};

[NumThreads(32, 1, 1)]
void mainAS(uint dtid : SV_DispatchThreadID, uint gtid : SV_GroupThreadID, uint gid : SV_GroupID)
{
    Payload p;
    p.meshletIndex = gid;
    
    DispatchMesh(1, 1, 1, p);
    
    
}


[NumThreads(128, 1, 1)]
[OutputTopology("triangle")]
void mainMS(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    in payload Payload payload,
    out indices uint3 tris[126],
    out vertices VertexOut verts[64]
)
{
    uint meshletIndex = payload.meshletIndex;
    MeshUniform meshInfo = Meshes[(MeshIndex)];
    
    StructuredBuffer<Vertex1P> verticesCoordinates = ResourceDescriptorHeap[(meshInfo.VertexBufferIndex)];
    StructuredBuffer<Vertex1N1UV1T> verticesProperties = ResourceDescriptorHeap[(meshInfo.VertexPropertiesBufferIndex)];
    ByteAddressBuffer uniqueVertexIndices = ResourceDescriptorHeap[(meshInfo.UniqueVertexIndexBufferIndex)];
    StructuredBuffer<uint> primitiveIndices = ResourceDescriptorHeap[(meshInfo.PrimitiveIndexBufferIndex)];

    
    StructuredBuffer<Meshlet> meshlets = ResourceDescriptorHeap[(meshInfo.MeshletBufferIndex)];
 
    Meshlet m = meshlets[(meshletIndex)];

    SetMeshOutputCounts(m.VertCount, m.PrimCount);

    if (gtid < m.PrimCount)
    {
        tris[gtid] = GetPrimitive(m, gtid, primitiveIndices);
    }

    if (gtid < m.VertCount)
    {
        uint vertexIndex = GetVertexIndex(m, gtid, uniqueVertexIndices, meshInfo.IndexSize);
        VertexOut vout = GetVertexAttributes(meshletIndex, vertexIndex, meshInfo, verticesCoordinates, verticesProperties, gtid);
        vout.lod = m.Lod;
        vout.group = m.GroupId;
        verts[gtid] = vout;
    }
}
