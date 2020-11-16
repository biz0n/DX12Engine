#pragma once

#include <Types.h>

#include <ShaderTypes.h>

#include <vector>

namespace Engine::Render
{
    class ConstantBufferUploader
    {
    public:
        ConstantBufferUploader(/* args */);
        ~ConstantBufferUploader();

        void UploadFrameUniform(uint32 rootParameterIndex, const FrameUniform& frameUniform);

        void UploadMeshUniform(uint32 rootParameterIndex, const dx::XMMATRIX &world);

        void UploadLightsUniform(uint32 rootParameterIndex, const std::vector<LightUniform>& lightsUniform);

        void UploadMaterialUniform(uint32 rootParameterIndex, const MaterialUniform& materialUniform);
    };
    
} // namespace Engine::Render
