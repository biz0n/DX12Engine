Texture2D colorTexture : register(t0);

float4 mainPS( float4 position : SV_Position ) : SV_Target0
{
    float4 color = colorTexture[(int2)position.xy];
    color = pow(color, 1.0/2.2);  

    return color;
}