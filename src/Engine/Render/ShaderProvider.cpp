#include "ShaderProvider.h"

#include <HAL/ShaderCompiler.h>
#include <StringUtils.h>

#include <HAL/ShaderCreationInfo.h>

namespace Engine::Render
{
    ShaderProvider::ShaderProvider() = default;
    
    ShaderProvider::~ShaderProvider() = default;

    ComPtr<ID3DBlob> ShaderProvider::GetShader(const HAL::ShaderCreationInfo &creationInfo)
    {
        size_t hash = std::hash<HAL::ShaderCreationInfo>{}(creationInfo);
        auto iter = mShadersMap.find(hash);

        if (iter != mShadersMap.end())
        {
            return iter->second;
        }
        else
        {
            auto shader = HAL::ShaderCompiler::Compile(
                StringToWString(creationInfo.path),
                creationInfo.defines.data(),
                creationInfo.entryPoint,
                creationInfo.target);
                
            mShadersMap.emplace(hash, shader);

            return shader;
        }
    }
} // namespace Engine::Render