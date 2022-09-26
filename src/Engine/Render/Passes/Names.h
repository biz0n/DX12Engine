#pragma once

#include <Types.h>
#include <Name.h>


namespace Engine::Render::Passes
{
    namespace Shaders
    {
        inline std::string ForwardPS {"Shaders\\Forward.hlsl"};
        inline std::string ForwardVS {"Shaders\\Forward.hlsl"};

        inline std::string ForwardMS{ "Shaders\\Mesh.hlsl" };

        inline std::string CubePS {"Shaders\\CubeMap.hlsl"};
        inline std::string CubeVS {"Shaders\\CubeMap.hlsl"};

        inline std::string ToneMapPS {"Shaders\\TonemappingPS.hlsl"};
        inline std::string TonemapVS {"Shaders\\ScreenVS.hlsl"};
        inline std::string ToneMapCS {"Shaders\\TonemappingPS.hlsl"};

        inline std::string DepthPS {"Shaders\\Depth.hlsl"};
        inline std::string DepthVS {"Shaders\\Depth.hlsl"};

        inline std::string BackBufferPS {"Shaders\\BackBufferPS.hlsl"};
        inline std::string BackBufferVS {"Shaders\\ScreenVS.hlsl"};
    }

    namespace ResourceNames
    {
        inline Name ForwardOutput {"ForwardOutput"};
        inline Name CubeOutput {"CubeOutput"};
        inline Name ForwardDepth {"ForwardDepth"};
        inline Name ShadowDepth {"Depth::Shadow"};
        inline Name TonemappingOutput {"Tonemaping::Output"};
        inline Name VisibilityOutput { "VisibilityOutput" };
    }

    namespace PSONames
    {
        inline Name ForwardCullBack {"Forward::PSO::CullModeBack"};
        inline Name ForwardCullNone {"Forward::PSO::CullModeNone"};

        inline Name ToneMapping {"ToneMapping::PSO"};
        inline Name BackBuffer {"BackBuffer::PSO"};

        inline Name Cube {"Cube::PSO"};
        inline Name Depth {"Depth::PSO"};
    }

    namespace RootSignatureNames
    {
        inline Name Forward {"Forward::RS"};
        inline Name ToneMapping {"ToneMapping::RS"};
        inline Name Cube {"Cube::RS"};
        inline Name Depth {"Depth::RS"};
        inline Name BackBuffer {"BackBuffer::RS"};
    }
    
} // namespace Engine::Render::Passes
