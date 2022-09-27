#pragma once

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <Types.h>
#include <Exceptions.h>

#include <dxcapi.h>
#include <filesystem>


namespace Engine::HAL
{
    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler() = default;

        ComPtr<IDxcBlob> Compile(
            const std::filesystem::path& path,
            const std::string& entrypoint,
            const std::string& target,
            const std::vector<std::string>& defines);

    private:
        ComPtr<IDxcLibrary> mLibrary;
        ComPtr<IDxcCompiler3> mCompiler;
    };

} // namespace Engine