#ifndef _VERTEX_
#define _VERTEX_

struct Vertex1P1N1UV1T
{
    float3 PositionL : POSITION;
    float3 NormalL : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};

#endif