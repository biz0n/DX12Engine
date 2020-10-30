#pragma once

#include <Types.h>
#include <Render/ShaderCreationInfo.h>
#include <unordered_map>


namespace Engine::Render
{
    class ShaderProvider
    {
    public:
        ShaderProvider();
        ~ShaderProvider();

        ComPtr<ID3DBlob> GetShader(const ShaderCreationInfo& creationInfo);

    private:
        std::unordered_map<size_t, ComPtr<ID3DBlob>> mShadersMap;
    };
} // namespace Engine::Render