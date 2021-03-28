#pragma once

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <Types.h>
#include <Exceptions.h>

namespace Engine
{
    class ShaderCompiler
    {
    public:
        static Microsoft::WRL::ComPtr<ID3DBlob> Compile(
            const std::wstring &filename,
            const D3D_SHADER_MACRO *defines,
            const std::string &entrypoint,
            const std::string &target)
        {
            UINT compileFlags = D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined(DEBUG) || defined(_DEBUG)
            compileFlags = compileFlags | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            HRESULT hr = S_OK;

            ComPtr<ID3DBlob> byteCode = nullptr;
            ComPtr<ID3DBlob> errors;
            hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

            if (errors != nullptr)
            {
                OutputDebugStringA((char *)errors->GetBufferPointer());
            }

            ThrowIfFailed(hr);

            return byteCode;
        }
    };

} // namespace Engine