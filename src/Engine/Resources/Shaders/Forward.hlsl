#include "ShaderTypes.h"
#include "LightUtils.hlsl"
 
ConstantBuffer<MeshUniform> ObjectCB : register(b0);

ConstantBuffer<FrameUniform> FrameCB : register(b1);

ConstantBuffer<MaterialUniform> MaterialCB : register(b2);

StructuredBuffer<LightUniform> Lights : register(t0, space1);

Texture2D baseColorTexture : register(t0);

Texture2D metallicRoughnessTexture : register(t1);

Texture2D normalTexture : register(t2);

Texture2D emissiveTexture : register(t3);

Texture2D occlusionTexture : register(t4);

SamplerState gsamPointWrap : register(s0);
 
struct VertexPosColor
{
    float3 PositionL : POSITION;
    float3 NormalL : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float4 Tangent : TANGENT;
};
 
struct VertexShaderOutput
{
    float3 PositionW : POSITION;
    float3 NormalW : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float3x3 TBN : TBN;
    float4 PositionH : SV_Position;
};
 
struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
};

VertexShaderOutput mainVS(VertexPosColor IN)
{
    VertexShaderOutput OUT;
 
    float4 posW = mul(float4(IN.PositionL, 1.0f), ObjectCB.World);
    float3 normalW = mul(IN.NormalL, (float3x3)ObjectCB.InverseTranspose);

    float3 T = normalize(mul(IN.Tangent.xyz, (float3x3)ObjectCB.World));
    float3 N = normalize(mul(IN.NormalL, (float3x3)ObjectCB.World));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * IN.Tangent.w;

    float3x3 TBN = float3x3(T, B, N);

    OUT.NormalW = normalW;
    OUT.PositionW = posW.xyz;
    OUT.PositionH = mul(posW, FrameCB.ViewProj);
    OUT.TBN = TBN;
    OUT.TextureCoord = IN.TextureCoord;
 
    return OUT;
}

PixelShaderOutput mainPS(VertexShaderOutput IN)
{
    float4 baseColor = MaterialCB.BaseColor;
    if (MaterialCB.HasBaseColorTexture)
    {
        baseColor = baseColorTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }

    clip(baseColor.a - MaterialCB.Cutoff);

    float3 N;
    if (MaterialCB.HasNormalTexture)
    {
        float3 n = normalTexture.Sample(gsamPointWrap, IN.TextureCoord).rgb;
        n = float3(n.r, 1-n.g, n.b);
        float scale = MaterialCB.NormalScale;
        N = (n * 2.0 - 1.0) * float3(scale, scale, 1.0);
        N = normalize(mul(N, IN.TBN));
    }
    else
    {
        N = normalize(IN.NormalW);
    }

    float metallic = MaterialCB.MetallicFactor;
    float roughness = MaterialCB.RoughnessFactor;
    if (MaterialCB.HasMetallicRoughnessTexture)
    {
        float4 metallicRoughness = metallicRoughnessTexture.Sample(gsamPointWrap, IN.TextureCoord);

        metallic = metallic * metallicRoughness.b;
        roughness = roughness * clamp(metallicRoughness.g, 0.04, 1.0);
    }

    float4 emissiveFactor = MaterialCB.EmissiveFactor;
    if (MaterialCB.HasEmissiveTexture)
    {
        emissiveFactor *= emissiveTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }

    float4 occlusion = MaterialCB.Ambient;
    if (MaterialCB.HasOcclusionTexture)
    {
        occlusion = occlusionTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }
    
    float3 V = normalize(FrameCB.EyePos - IN.PositionW);

    float3 F0 = 0.04; 
    F0 = lerp(F0, baseColor.rgb, metallic);
	           
    float3 directLuminance = 0.0f;

    for(int i = 0; i < FrameCB.LightsCount; ++i) 
    {
        LightUniform light = Lights[i];

        if ( !light.Enabled ) continue;

        float3 luminance = 0.0f;
        switch( light.LightType )
        {
        case DIRECTIONAL_LIGHT:
            {
                luminance = ApplyDirectionalLight(light, IN.PositionW, F0, N, V, baseColor.rgb, metallic, roughness);
            }
            break;
        case POINT_LIGHT: 
            {
                luminance = ApplyPointLight(light, IN.PositionW, F0, N, V, baseColor.rgb, metallic, roughness);
            }
            break;
        case SPOT_LIGHT:
            {
                luminance = ApplySpotLight(light, IN.PositionW, F0, N, V, baseColor.rgb, metallic, roughness);
            }
            break;
        }

        directLuminance += luminance;
    }   

    float3 ambient = 0.03 * baseColor.rgb * occlusion.rgb;
    float3 color = emissiveFactor.rgb + ambient + directLuminance;
	
    color = color / (color + 1.0f);
    color = pow(color, 1.0/2.2);  
     
 
    float4 finalColor = float4(color, baseColor.a);
 
    PixelShaderOutput output;
    output.Color = finalColor;
    return output;
}