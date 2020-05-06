#pragma once

#include <d3d12.h>
#include <D3Dcompiler.h>
#include <Types.h>
#include <Exceptions.h>

namespace Engine
{
    class Utils
    {
    public:
        static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
            const std::wstring &filename,
            const D3D_SHADER_MACRO *defines,
            const std::string &entrypoint,
            const std::string &target)
        {
            UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
            compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            HRESULT hr = S_OK;

            ComPtr<ID3DBlob> byteCode = nullptr;
            ComPtr<ID3DBlob> errors;
            hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

            if (errors != nullptr)
                OutputDebugStringA((char *)errors->GetBufferPointer());

            ThrowIfFailed(hr);

            return byteCode;
        }

        static std::wstring ToWide(const std::string &narrow)
        {
            wchar_t wide[512];
            mbstowcs_s(nullptr, wide, narrow.c_str(), _TRUNCATE);
            return wide;
        }
    };

} // namespace Engine