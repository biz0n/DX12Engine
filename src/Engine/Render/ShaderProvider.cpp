#include "ShaderProvider.h"

#include <Utils.h>

namespace Engine::Render
{
    ShaderProvider::ShaderProvider() = default;
    
    ShaderProvider::~ShaderProvider() = default;

    ComPtr<ID3DBlob> ShaderProvider::GetShader(const ShaderCreationInfo &creationInfo)
    {
        size_t hash = std::hash<ShaderCreationInfo>{}(creationInfo);
        auto iter = mShadersMap.find(hash);

        if (iter != mShadersMap.end())
        {
            return iter->second;
        }
        else
        {
            auto shader = Utils::CompileShader(
                Utils::ToWide(creationInfo.path),
                creationInfo.defines.data(),
                creationInfo.entryPoint,
                creationInfo.target);
                
            mShadersMap.emplace(hash, shader);

            return shader;
        }
    }
} // namespace Engine::Render