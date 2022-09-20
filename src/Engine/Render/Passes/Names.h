#pragma once

#include <Types.h>
#include <Name.h>


namespace Engine::Render::Passes
{
    namespace Shaders
    {
        inline std::string ForwardPS {"Render\\Shaders\\Forward.hlsl"};
        inline std::string ForwardVS {"Render\\Shaders\\Forward.hlsl"};

        inline std::string CubePS {"Render\\Shaders\\CubeMap.hlsl"};
        inline std::string CubeVS {"Render\\Shaders\\CubeMap.hlsl"};

        inline std::string ToneMapPS {"Render\\Shaders\\TonemappingPS.hlsl"};
        inline std::string TonemapVS {"Render\\Shaders\\ScreenVS.hlsl"};
        inline std::string ToneMapCS {"Render\\Shaders\\TonemappingPS.hlsl"};

        inline std::string DepthPS {"Render\\Shaders\\Depth.hlsl"};
        inline std::string DepthVS {"Render\\Shaders\\Depth.hlsl"};

        inline std::string BackBufferPS {"Render\\Shaders\\BackBufferPS.hlsl"};
        inline std::string BackBufferVS {"Render\\Shaders\\ScreenVS.hlsl"};
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
