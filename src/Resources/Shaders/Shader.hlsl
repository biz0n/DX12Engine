#include "ShaderTypes.h"
#include "LightUtils.hlsl"
 
ConstantBuffer<MeshUniform> ObjectCB : register(b0);

ConstantBuffer<FrameUniform> FrameCB : register(b1);

ConstantBuffer<MaterialUniform> MaterialCB : register(b2);

Texture2D diffuseTexture : register(t0);

Texture2D metallicRoughnessTexture : register(t1);

Texture2D normalTexture : register(t2);

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

float4 mainPS(VertexShaderOutput IN) : SV_TARGET
{
    LightingResult lit = ComputeLighting( 
        FrameCB.LightProperties, 
        float4(FrameCB.EyePos, 1.0f), 
        float4(IN.PositionW, 1.0f), 
        normalize(IN.NormalW), 
        MaterialCB.SpecularPower );
     
     float4 albedo = diffuseTexture.Sample(gsamPointWrap, IN.TextureCoord);

     float3 n = normalTexture.Sample(gsamPointWrap, IN.TextureCoord).rgb;
     n = float3(n.r, 1-n.g, n.b);
     float scale = MaterialCB.NormalScale;
     float3 N = (n * 2.0 - 1.0) * float3(scale, scale, 1.0);
     N = normalize(mul(N, IN.TBN));
     float4 metallicRoughness = metallicRoughnessTexture.Sample(gsamPointWrap, IN.TextureCoord);
     float4 c = ComputeLighting2(
         FrameCB.LightProperties,
         float4(FrameCB.EyePos, 1.0f),
         float4(IN.PositionW, 1.0f),
         N,
        // normalize(IN.NormalW),
         metallicRoughness.r,
         clamp(metallicRoughness.g, 0.04, 1.0),
         albedo.rgb,
         MaterialCB.Ambient
     );
    
    float4 d = float4(1.0f, 1.0f, 1.0f, 1.0f);// diffuseTexture.Sample(gsamPointWrap, IN.TextureCoord);
    float4 emissive = MaterialCB.Emissive;
    float4 ambient = MaterialCB.Ambient * float4(FrameCB.LightProperties.GlobalAmbient, 1.0f);
    float4 diffuse = MaterialCB.Diffuse * lit.Diffuse;
    float4 specular = MaterialCB.Specular * lit.Specular;
 
    float4 texColor = d;
     
 
    float4 finalColor = c;//( emissive + ambient + diffuse + specular ) * texColor;
    finalColor.a = albedo.a;
 
    return finalColor;
}