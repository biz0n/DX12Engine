#pragma once

#include <Types.h>
#include <Name.h>


namespace Engine::Render::Passes
{
    namespace Shaders
    {
        inline String ForwardPS {"Resources\\Shaders\\Forward.hlsl"};
        inline String ForwardVS {"Resources\\Shaders\\Forward.hlsl"};

        inline String CubePS {"Resources\\Shaders\\CubeMap.hlsl"};
        inline String CubeVS {"Resources\\Shaders\\CubeMap.hlsl"};

        inline String ToneMapPS {"Resources\\Shaders\\TonemappingPS.hlsl"};
        inline String TonemapVS {"Resources\\Shaders\\ScreenVS.hlsl"};

        inline String DepthPS {"Resources\\Shaders\\Depth.hlsl"};
        inline String DepthVS {"Resources\\Shaders\\Depth.hlsl"};
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
