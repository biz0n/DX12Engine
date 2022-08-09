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

        ComPtr<ID3DBlob> Compile(
            const std::wstring& filename,
            const D3D_SHADER_MACRO* defines,
            const std::string& entrypoint,
            const std::string& target);

        ComPtr<IDxcBlob> Compile2(
            const std::wstring& filename,
            const D3D_SHADER_MACRO* defines,
            const std::string& entrypoint,
            const std::string& target);

    private:
        ComPtr<IDxcLibrary> mLibrary;
        ComPtr<IDxcCompiler3> mCompiler;
    };

} // namespace Engine