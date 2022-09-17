#include "ShaderTypes.h"
#include "LightUtils.hlsl"
#include "Vertex.hlsl"
 
cbuffer Mesh : register(b0)
{
    int MeshIndex;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

StructuredBuffer<MeshUniform> Meshes : register(t0, space1);

StructuredBuffer<LightUniform> Lights : register(t1, space1);

StructuredBuffer<MaterialUniform> Materials : register(t2, space1);

#include "BaseLayout.hlsl"

struct VertexShaderOutput
{
    float3 PositionW : POSITION0;
    float4 ShadowPosH : POSITION1;
    float3 NormalW : NORMAL;
    float2 TextureCoord : TEXCOORD;
    float3x3 TBN : TBN;
    float4 PositionH : SV_Position;
    uint indexId : INDEX;
};
 
struct PixelShaderOutput
{
    float4 Color : SV_TARGET0;
    float4 VisibilityBuffer : SV_Target1;
};

float3 SRGBToLinear(float3 sRGBCol)
{
    float3 linearRGBLo = sRGBCol / 12.92;
    float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, 2.4);
    float3 linearRGB = (sRGBCol <= 0.04045) ? linearRGBLo : linearRGBHi;
    return linearRGB;
}

float ShadowCalculation(float4 fragPosLightSpace, int shadowIndex)
{
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    float currentDepth = projCoords.z;


    uint width, height, numMips;
    Texture2D<float4> shadowTexture = ResourceDescriptorHeap[shadowIndex];
    shadowTexture.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += shadowTexture.SampleCmpLevelZero(gsamShadow,
            projCoords.xy + offsets[i], currentDepth).r;
    }
    
    return percentLit / 9.0f;
}

float4 unpackUnorm4x8(uint p)
{
    return float4(float(p & 0x000000FF) / 255.0,
				  float((p & 0x0000FF00) >> 8) / 255.0,
				  float((p & 0x00FF0000) >> 16) / 255.0,
				  float((p & 0xFF000000) >> 24) / 255.0);
}

VertexShaderOutput mainVS(uint indexId : SV_VertexID)
{
    MeshUniform ObjectCB = Meshes[MeshIndex];
    StructuredBuffer<Vertex1P> verticesCoordinates = ResourceDescriptorHeap[ObjectCB.VertexBufferIndex];
    StructuredBuffer<Vertex1N1UV1T> verticesProperties = ResourceDescriptorHeap[ObjectCB.VertexPropertiesBufferIndex];
    StructuredBuffer<uint> indices = ResourceDescriptorHeap[ObjectCB.IndexBufferIndex];

    uint vertexId = indices[indexId];

    Vertex1P IN = verticesCoordinates[vertexId];
    Vertex1N1UV1T properties = verticesProperties[vertexId];


    VertexShaderOutput OUT;
 
    float4 posW = mul(float4(IN.PositionL, 1.0f), ObjectCB.World);
    float3 normalW = mul(properties.NormalL, (float3x3) ObjectCB.InverseTranspose);

    float3 T = normalize(mul(properties.Tangent.xyz, (float3x3) ObjectCB.World));
    float3 N = normalize(mul(properties.NormalL, (float3x3) ObjectCB.World));
    T = normalize(T - dot(T, N) * N);
    float3 B = cross(N, T) * properties.Tangent.w;

    float3x3 TBN = float3x3(T, B, N);

    OUT.NormalW = normalW;
    OUT.PositionW = posW.xyz;
    OUT.ShadowPosH = mul(posW, FrameCB.ShadowTransform);
    OUT.PositionH = mul(posW, FrameCB.ViewProj);
    OUT.TBN = TBN;
    OUT.TextureCoord = properties.TextureCoord;
    OUT.indexId = indexId;
 
    return OUT;
}

PixelShaderOutput mainPS(VertexShaderOutput IN)
{
    MaterialUniform MaterialCB = Materials[Meshes[MeshIndex].MaterialIndex];
    
    float4 baseColor = MaterialCB.BaseColor;
    if (MaterialCB.HasBaseColorTexture)
    {
        Texture2D<float4> baseColorTexture = ResourceDescriptorHeap[MaterialCB.BaseColorIndex];
        baseColor = baseColorTexture.Sample(gsamPointWrap, IN.TextureCoord);
        baseColor = float4(SRGBToLinear(baseColor.rgb), baseColor.a);
    }

    clip(baseColor.a - MaterialCB.Cutoff);

    float3 N;
    if (MaterialCB.HasNormalTexture)
    {
        Texture2D<float4> normalTexture = ResourceDescriptorHeap[MaterialCB.NormalIndex];
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
        Texture2D<float4> metallicRoughnessTexture = ResourceDescriptorHeap[MaterialCB.MetallicRoughnessIndex];
        float4 metallicRoughness = metallicRoughnessTexture.Sample(gsamPointWrap, IN.TextureCoord);

        metallic = metallic * metallicRoughness.b;
        roughness = roughness * clamp(metallicRoughness.g, 0.04, 1.0);
    }

    float4 emissiveFactor = MaterialCB.EmissiveFactor;
    if (MaterialCB.HasEmissiveTexture)
    {
        Texture2D<float4> emissiveTexture = ResourceDescriptorHeap[MaterialCB.EmissiveIndex];
        float4 emissiveColor = emissiveTexture.Sample(gsamPointWrap, IN.TextureCoord);
        emissiveColor = float4(SRGBToLinear(emissiveColor.rgb), emissiveColor.a);
        emissiveFactor *= emissiveColor;
    }

    float4 occlusion = MaterialCB.Ambient;
    if (MaterialCB.HasOcclusionTexture)
    {
        Texture2D<float4> occlusionTexture = ResourceDescriptorHeap[MaterialCB.OcclusionIndex];
        occlusion = occlusionTexture.Sample(gsamPointWrap, IN.TextureCoord);
    }
    
    float3 V = normalize(FrameCB.EyePos - IN.PositionW);

    float3 F0 = 0.04; 
    F0 = lerp(F0, baseColor.rgb, metallic);
	           
    float3 directLuminance = 0.0f;

    for(int i = 0; i < FrameCB.LightsCount; ++i) 
    {
        LightUniform light = Lights[i];

        float3 luminance = 0.0f;
        switch( light.LightType )
        {
        case DIRECTIONAL_LIGHT:
            {
                luminance = ApplyDirectionalLight(light, IN.PositionW, F0, N, V, baseColor.rgb, metallic, roughness);

                if (FrameCB.HasShadowTexture)
                {
                    luminance *= ShadowCalculation(IN.ShadowPosH, FrameCB.ShadowIndex);
                }
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

    float3 ambient = 0.03 * baseColor.rgb * occlusion.r;
    float3 color = emissiveFactor.rgb + ambient + directLuminance;
 
    float4 finalColor = float4(color, baseColor.a);
 
    PixelShaderOutput output;
    output.Color = finalColor;
    
    uint meshIdIndexId = ((MeshIndex << 23) & 0x7F800000) | (IN.indexId & 0x007FFFFF);
    float f = (float) IN.indexId;
    
    float4 cc = float4(
            float(IN.indexId & 1),
            float(IN.indexId & 5) / 4,
            float(IN.indexId & 9) / 8, 1);
    
    output.VisibilityBuffer = cc;//
    //half4(sin(f / 10), sin(f / 100), sin(f / 1000), 1) * 0.3 + 0.7;
    return output;
}