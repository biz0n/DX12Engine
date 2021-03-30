#pragma once

#include <Types.h>
#include <Name.h>


namespace Engine::Render::Passes
{
    namespace Shaders
    {
        inline String ForwardPS {"Render\\Shaders\\Forward.hlsl"};
        inline String ForwardVS {"Render\\Shaders\\Forward.hlsl"};

        inline String CubePS {"Render\\Shaders\\CubeMap.hlsl"};
        inline String CubeVS {"Render\\Shaders\\CubeMap.hlsl"};

        inline String ToneMapPS {"Render\\Shaders\\TonemappingPS.hlsl"};
        inline String TonemapVS {"Render\\Shaders\\ScreenVS.hlsl"};

        inline String DepthPS {"Render\\Shaders\\Depth.hlsl"};
        inline String DepthVS {"Render\\Shaders\\Depth.hlsl"};
    }

    namespace ResourceNames
    {
        inline Name ForwardOutput {"ForwardOutput"};
        inline Name CubeOutput {"CubeOutput"};
        inline Name ForwardDepth {"ForwardDepth"};
        inline Name ShadowDepth {"Depth::Shadow"};
    }

    namespace PSONames
    {
        inline Name ForwardCullBack {"Forward::PSO::CullModeBack"};
        inline Name ForwardCullNone {"Forward::PSO::CullModeNone"};

        inline Name ToneMapping {"ToneMapping::PSO"};

        inline Name Cube {"Cube::PSO"};
        inline Name Depth {"Depth::PSO"};
    }

    namespace RootSignatureNames
    {
        inline Name Forward {"Forward::RS"};
        inline Name ToneMapping {"ToneMapping::RS"};
        inline Name Cube {"Cube::RS"};
        inline Name Depth {"Depth::RS"};
    }
    
} // namespace Engine::Render::Passes
