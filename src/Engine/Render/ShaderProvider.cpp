#include "ShaderProvider.h"

#include <HAL/ShaderCompiler.h>

#include <HAL/ShaderCreationInfo.h>

#include <PathResolver.h>

namespace Engine::Render
{
    ShaderProvider::ShaderProvider()
    {
        mShaderCompiler = MakeUnique<HAL::ShaderCompiler>();
    }
    
    ShaderProvider::~ShaderProvider() = default;

    D3D12_SHADER_BYTECODE ShaderProvider::GetShader(const HAL::ShaderCreationInfo &creationInfo)
    {
        size_t hash = std::hash<HAL::ShaderCreationInfo>{}(creationInfo);
        auto iter = mShadersMap.find(hash);

        if (iter != mShadersMap.end())
        {
            return { iter->second->GetBufferPointer(), iter->second->GetBufferSize() };
        }
        else
        {
            auto shader = mShaderCompiler->Compile(
                PathResolver::GetShaderPath(creationInfo.path).string(),
                creationInfo.entryPoint,
                creationInfo.target,
                creationInfo.defines);
                
            mShadersMap.emplace(hash, shader);

            return { shader->GetBufferPointer(), shader->GetBufferSize() };
        }
    }
} // namespace Engine::Render