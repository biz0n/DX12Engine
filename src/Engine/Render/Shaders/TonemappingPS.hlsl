Texture2D colorTexture : register(t0);

#include "BaseLayout.hlsl"

float4 mainPS( float4 position : SV_Position ) : SV_Target0
{
    float4 color = colorTexture[(int2)position.xy];
   // float3 mapped = color.xyz / (color.xyz + float3(1.0, 1.0, 1.0));
   float3 mapped = float3(1.0, 1.0, 1.0) - exp(-color.xyz * 1);

    color = pow(float4(mapped, 1.0), 1.0/2.2);  

    return color;
}