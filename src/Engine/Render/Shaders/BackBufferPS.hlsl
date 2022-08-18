cbuffer ColorTexture : register(b0)
{
    int ColorTextureIndex;
};

#include "BaseLayout.hlsl"

float4 mainPS( float4 position : SV_Position ) : SV_Target0
{
    Texture2D<float4> colorTexture = ResourceDescriptorHeap[ColorTextureIndex];
    float4 color = colorTexture[(int2)position.xy];

    return color;
}