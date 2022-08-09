#pragma once

#include <Types.h>
#include <Render/RenderForwards.h>
#include <HAL/HALForwards.h>
#include <unordered_map>

#include <d3d12.h>
#include <dxcapi.h>

namespace Engine::Render
{
    class ShaderProvider
    {
    public:
        ShaderProvider();
        ~ShaderProvider();

        D3D12_SHADER_BYTECODE GetShader(const HAL::ShaderCreationInfo& creationInfo);

    private:
        std::unordered_map<size_t, ComPtr<IDxcBlob>> mShadersMap;
        UniquePtr<HAL::ShaderCompiler> mShaderCompiler;
    };
} // namespace Engine::Render