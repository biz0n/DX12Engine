#include "ShaderTypes.h"

struct LightingResult
{
    float4 Diffuse;
    float4 Specular;
};

float4 DoDiffuse( LightUniform light, float3 L, float3 N )
{
    float NdotL = max(0.0f, dot(N, L));
    return light.Color * NdotL;
}

float4 DoSpecular( LightUniform light, float3 V, float3 L, float3 N, float specularPower )
{
    // Phong lighting.
    float3 R = normalize( reflect( -L, N ) );
    float RdotV = max( 0, dot( R, V ) );
 
    // Blinn-Phong lighting
    float3 H = normalize( L + V );
    float NdotH = max( 0, dot( N, H ) );
 
    return light.Color * pow( RdotV, specularPower );
}

float DoAttenuation( LightUniform light, float d )
{
    return 1.0f / ( light.ConstantAttenuation + light.LinearAttenuation * d + light.QuadraticAttenuation * d * d );
}

float DoSpotCone( LightUniform light, float3 L )
{
    float minCos = cos( light.SpotAngle );
    float maxCos = ( minCos + 1.0f ) / 2.0f;
    float cosAngle = dot( light.DirectionWS.xyz, -L );
    return smoothstep( minCos, maxCos, cosAngle ); 
}

LightingResult DoPointLight( LightUniform light, float3 V, float4 P, float3 N, float specularPower )
{
    LightingResult result;
 
    float3 L = ( light.PositionWS - P ).xyz;
    float distance = length(L);
    L = L / distance;
 
    float attenuation = DoAttenuation( light, distance );
 
    result.Diffuse = DoDiffuse( light, L, N ) * attenuation;
    result.Specular = DoSpecular( light, V, L, N, specularPower ) * attenuation;
 
    return result;
}

LightingResult DoDirectionalLight( LightUniform light, float3 V, float4 P, float3 N, float specularPower )
{
    LightingResult result;
 
    float3 L = -light.DirectionWS.xyz;
 
    result.Diffuse = DoDiffuse( light, L, N );
    result.Specular = DoSpecular( light, V, L, N, specularPower );
 
    return result;
}

LightingResult DoSpotLight( LightUniform light, float3 V, float4 P, float3 N, float specularPower )
{
    LightingResult result;
 
    float3 L = ( light.PositionWS - P ).xyz;
    float distance = length(L);
    L = L / distance;
 
    float attenuation = DoAttenuation( light, distance );
    float spotIntensity = DoSpotCone( light, L );
 
    result.Diffuse = DoDiffuse( light, L, N ) * attenuation * spotIntensity;
    result.Specular = DoSpecular( light, V, L, N, specularPower ) * attenuation * spotIntensity;
 
    return result;
}

LightingResult ComputeLighting( LightProperties lightProperties, float4 eyePosition, float4 P, float3 N, float specularPower )
{
    float3 V = normalize( eyePosition - P).xyz;
 
    LightingResult totalResult = { {0, 0, 0, 0}, {0, 0, 0, 0} };
 
    [unroll]
    for( int i = 0; i < lightProperties.LightsCount; ++i )
    {
        LightingResult result = { {0, 0, 0, 0}, {0, 0, 0, 0} };

        LightUniform light = lightProperties.Lights[i];
 
        if ( !light.Enabled ) continue;
         
        switch( light.LightType )
        {
        case DIRECTIONAL_LIGHT:
            {
                result = DoDirectionalLight( light, V, P, N, specularPower );
            }
            break;
        case POINT_LIGHT: 
            {
                result = DoPointLight( light, V, P, N, specularPower );
            }
            break;
        case SPOT_LIGHT:
            {
                result = DoSpotLight( light, V, P, N, specularPower );
            }
            break;
        }
        totalResult.Diffuse += result.Diffuse;
        totalResult.Specular += result.Specular;
    }
 
    totalResult.Diffuse = saturate(totalResult.Diffuse);
    totalResult.Specular = saturate(totalResult.Specular);
 
    return totalResult;
}

static const float PI = 3.14159265359;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
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
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float4 ComputeLighting2( LightProperties lightProperties, float4 eyePosition, float4 P, float3 N, float metallic, float roughness, float3 albedo, float3 ao)
{
   // albedo = pow(albedo, 2.2);
    float3 V = normalize(eyePosition - P);

    float3 F0 = 0.04; 
    F0 = lerp(F0, albedo, metallic);
	           
    // reflectance equation
    float3 Lo = 0.0f;
    for(int i = 0; i < lightProperties.LightsCount; ++i) 
    {
        LightUniform light = lightProperties.Lights[i];

        if ( !light.Enabled ) continue;
        if ( light.LightType != POINT_LIGHT) continue;

        // calculate per-light radiance
        float3 L = normalize(light.PositionWS - P);
        float3 H = normalize(V + L);
        float distance    = length(light.PositionWS - P);
        float attenuation = 1.0 / (distance * distance);
        float3 radiance     = light.Color.rgb * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        float3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        float3 kS = F;
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        float3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        float3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    float3 ambient = 0.03 * albedo * ao;
    float3 color = ambient + Lo;
	
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, 1.0/2.2);  
   
    return float4(color, 1.0);
}