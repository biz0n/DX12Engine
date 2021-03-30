#pragma once

#include <Types.h>
#include <Render/RenderForwards.h>
#include <HAL/HALForwards.h>
#include <unordered_map>

#include <d3d12.h>

namespace Engine::Render
{
    class ShaderProvider
    {
    public:
        ShaderProvider();
        ~ShaderProvider();

        ComPtr<ID3DBlob> GetShader(const HAL::ShaderCreationInfo& creationInfo);

    private:
        std::unordered_map<size_t, ComPtr<ID3DBlob>> mShadersMap;
    };
} // namespace Engine::Render