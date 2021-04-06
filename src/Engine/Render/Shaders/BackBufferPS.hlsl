Texture2D colorTexture : register(t0);

#include "BaseLayout.hlsl"

float4 mainPS( float4 position : SV_Position ) : SV_Target0
{
    float4 color = colorTexture[(int2)position.xy];

    return color;
}