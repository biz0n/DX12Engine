#ifndef _VERTEX_
#define _VERTEX_

struct Vertex1P
{
    float3 PositionL : POSITION;
};

struct Vertex1N1UV1T
{
    float3 NormalL : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};

struct Meshlet
{
    uint VertCount;
    uint VertOffset;
    uint PrimCount;
    uint PrimOffset;
};

#endif