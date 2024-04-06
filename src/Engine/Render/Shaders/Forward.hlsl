#include "ShaderTypes.h"
#include "LightUtils.hlsl"
#include "Vertex.hlsl"
 
cbuffer Mesh : register(b0, space0)
{
    int MeshIndex;
};

cbuffer LodIndex : register(b0, space1)
{
    int Lod;
};

ConstantBuffer<FrameUniform> FrameCB : register(b1);

StructuredBuffer<MeshUniform> Meshes : register(t0, space1);

StructuredBuffer<LightUniform> Lights : register(t1, space1);

StructuredBuffer<MaterialUniform> Materials : register(t2, space1);

#include "BaseLayout.hlsl"

struct VertexShaderOutput
{
    float4 PositionH : SV_Position;
    float3 PositionW : POSITION0;
   // float4 ShadowPosH : POSITION1;
    float3 NormalW : NORMAL;
    float2 TextureCoord : TEXCOORD;
   // float3x3 TBN : TBN;
    
    uint indexId : INDEX;
    uint lod : INDEX1;
    uint group : INDEX2;
};

struct VertexOut
{
    float4 PositionHS : SV_Position;
    float3 PositionVS : POSITION0;
    float3 Normal : NORMAL0;
    uint MeshletIndex : COLOR0;
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
  //  OUT.ShadowPosH = mul(posW, FrameCB.ShadowTransform);
    OUT.PositionH = mul(posW, FrameCB.ViewProj);
  //  OUT.TBN = TBN;
  //  OUT.TextureCoord = properties.TextureCoord;
    OUT.indexId = indexId;
 
    return OUT;
}

// https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
float3 HSVToRGB(float h, float s, float v)
{
    float h_i = floor(h * 6.0f);
    float f = h * 6.0f - h_i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (int(h_i))
    {
        case 0:
            return float3(v, t, p);
        case 1:
            return float3(q, v, p);
        case 2:
            return float3(p, v, t);
        case 3:
            return float3(p, q, v);
        case 4:
            return float3(t, p, v);
        case 5:
            return float3(v, p, q);
        default:
            return float3(1.0f, 0.0f, 1.0f); // doesn't happen
    }
}

// This uses the golden ratio to make N hues that are maximally distant
// from each other for any N colors desired. Need to use indices [0,N)
// for this to work though. Just does a 1D low discrepancy sequence
// for hue, and has constant s and v values.
float3 IndexToColor(int index, float s = 0.5f, float v = 0.95f)
{
    static const float c_goldenRatioConjugate = 0.61803398875f;
    float h = frac(float(index) * c_goldenRatioConjugate);
    return HSVToRGB(h, s, v);
}

float FilteredCheckers(in float2 p)
{
    float2 dpdx = ddx(p);
    float2 dpdy = ddy(p);

    float2 w = max(abs(dpdx), abs(dpdy));
    float2 i = 2.0 * (abs(frac((p - 0.5 * w) * 0.5) - 0.5) -
                  abs(frac((p + 0.5 * w) * 0.5) - 0.5)) / w;
    return 0.5 - 0.5 * i.x * i.y;
}

float UnfilteredCheckers(in float2 p)
{
    p /= 2.0f;
    float x = frac(p.x) < 0.5f ? 0.0f : 1.0f;
    float y = frac(p.y) < 0.5f ? 0.0f : 1.0f;

    float ret = (x == y ? 0.0f : 1.0f);
    return ret;
}

PixelShaderOutput mainPS(VertexShaderOutput input)
{
    float ppp = input.lod == Lod ? 1 : -1;
    clip(ppp);
    
    float ambientIntensity = 0.1;
    float3 lightColor = float3(1, 1, 1);
    float3 lightDir = -normalize(float3(1, -1, 1));

    float3 diffuseColor;
    float shininess;
    float filteredCheckers = UnfilteredCheckers(input.TextureCoord);
    float3 cc = IndexToColor(input.group, 0.98f);
    
        uint meshletIndex = input.indexId;
        diffuseColor = cc;
        shininess = 32.0;

    float3 normal = normalize(input.NormalW);

    // Do some fancy Blinn-Phong shading!
    float cosAngle = saturate(dot(normal, lightDir));
    float3 viewDir = -normalize(input.PositionW);
    float3 halfAngle = normalize(lightDir + viewDir);

    float blinnTerm = saturate(dot(normal, halfAngle));
    blinnTerm = cosAngle != 0.0 ? blinnTerm : 0.0;
    blinnTerm = pow(blinnTerm, shininess);

    float3 finalColor = (cosAngle + blinnTerm + ambientIntensity) * diffuseColor;
    
    PixelShaderOutput output;
    output.Color = float4(finalColor, 1);
    
    output.VisibilityBuffer = float4(cc, 1);;
    return output;
    /*
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
*/
}