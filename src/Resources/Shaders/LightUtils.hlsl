#include "ShaderTypes.h"

static const float PI = 3.14159265359;

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 

float DistributionGGX(float NdotH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 Specular(float3 F, float NdotL, float NdotH, float NdotV, float roughness)
{
    // cook-torrance brdf
    float NDF = DistributionGGX(NdotH, roughness);        
    float G = GeometrySmith(NdotV, NdotL, roughness);      

    float3 numerator = NDF * G * F;
    float  denominator = 4.0 * NdotV * NdotL;
    float3 specular = numerator / max(denominator, 0.001);
    return specular;
}

float3 Luminance(float3 F0, float3 albedo, float3 N, float3 V, float3 L, float metallic, float roughness)
{
    float3 H = normalize(V + L);

    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F  = FresnelSchlick(HdotV, F0);       
        
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    float3 specular = Specular(F, NdotL, NdotH, NdotV, roughness);

    return (kD * albedo / PI + specular) * NdotL;
}

float CalculateAttenuation( LightUniform light, float distance )
{
    return 1.0f / ( light.ConstantAttenuation + light.LinearAttenuation * distance + light.QuadraticAttenuation * distance * distance );
}

float CalculateSpotCone( LightUniform light, float3 L )
{
    float lightAngleScale = 1.0f / max(0.001f, cos(light.InnerConeAngle) - cos(light.OuterConeAngle));
    float lightAngleOffset = -cos(light.OuterConeAngle) * lightAngleScale;
    float cosAngle = dot(light.DirectionWS, -L);
    float angularAttenuation = saturate(cosAngle * lightAngleScale + lightAngleOffset);
    return angularAttenuation * angularAttenuation;
}

float3 ApplyDirectionalLight(LightUniform light, float3 positionWS, float3 F0, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 L = -light.DirectionWS;
    float3 luminance = Luminance(F0, albedo, N, V, L, metallic, roughness);

    return luminance * light.Color;
}

float3 ApplyPointLight(LightUniform light, float3 positionWS, float3 F0, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 L = ( light.PositionWS - positionWS );
    float distance = length(L);
    L = L / distance;

    float attenuation = CalculateAttenuation(light, distance);

    float3 luminance = Luminance(F0, albedo, N, V, L, metallic, roughness);

    return luminance * light.Color * attenuation;
}

float3 ApplySpotLight(LightUniform light, float3 positionWS, float3 F0, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
    float3 L = ( light.PositionWS.xyz - positionWS );
    float distance = length(L);
    L = L / distance;

    float attenuation = CalculateAttenuation(light, distance);
    float spotIntensity = CalculateSpotCone( light, L );

    float3 luminance = Luminance(F0, albedo, N, V, L, metallic, roughness);

    return luminance * light.Color * attenuation * spotIntensity;
}
