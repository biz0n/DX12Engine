#include "ShaderCompiler.h"

#include <StringUtils.h>

#include <d3dcompiler.h>
#include <vector>
#include <string>

namespace Engine::HAL
{
    ShaderCompiler::ShaderCompiler()
    {
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(mLibrary.GetAddressOf())));
        ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(mCompiler.GetAddressOf())));
    }

    ComPtr<ID3DBlob> ShaderCompiler::Compile(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target)
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
            OutputDebugStringA((char*)errors->GetBufferPointer());
        }

        ThrowIfFailed(hr);

        return byteCode;
    }

    ComPtr<IDxcBlob> ShaderCompiler::Compile2(
        const std::wstring& filename,
        const D3D_SHADER_MACRO* defines,
        const std::string& entrypoint,
        const std::string& target)
    {
        std::wstring wEntryPoint = StringToWString(entrypoint);
        std::wstring wProfile = StringToWString(target);
        LPWSTR suggestedDebugName = nullptr;

        bool debugBuild = false;
#if defined(DEBUG) || defined(_DEBUG)
        debugBuild = true;
#endif

        ComPtr<IDxcUtils> pUtils;
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));
        ComPtr<IDxcBlobEncoding> pSource;

        ComPtr<IDxcIncludeHandler> pDefaultIncludeHandler;
      //  if (!pUtils)
        {
           // ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf())));
            ThrowIfFailed(pUtils->CreateDefaultIncludeHandler(pDefaultIncludeHandler.GetAddressOf()));
        }

        HRESULT br = mLibrary->CreateBlobFromFile(filename.c_str(), nullptr, pSource.GetAddressOf());

        std::vector<std::wstring> arguments;

        arguments.push_back(DXC_ARG_ALL_RESOURCES_BOUND);
        //-E for the entry point (eg. PSMain)
        arguments.push_back(L"-E");
        arguments.push_back(wEntryPoint);

        arguments.push_back(L"-I");
        auto inc = std::filesystem::path{ filename }.parent_path();
        arguments.push_back(inc.wstring());

        //-T for the target profile (eg. ps_6_2)
        arguments.push_back(L"-T");
        arguments.push_back(wProfile);


        arguments.push_back(L"-Qstrip_reflect");

        if (debugBuild)
        {
            arguments.push_back(L"-Qembed_debug");
            arguments.push_back(DXC_ARG_DEBUG); //-Zi
        }
        else
        {
            arguments.push_back(L"-Qstrip_debug");
        }
        

        arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
        

      //  for (const std::wstring& define : defines)
        {
      //      arguments.push_back(L"-D");
      //      arguments.push_back(define.c_str());
        }

        std::vector<LPCWSTR> argumentPtrs;

        for (auto& argument : arguments)
        {
            argumentPtrs.push_back(argument.c_str());
        }

        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = pSource->GetBufferPointer();
        sourceBuffer.Size = pSource->GetBufferSize();
        sourceBuffer.Encoding = 0;

        ComPtr<IDxcResult> pCompileResult;
        mCompiler->Compile(&sourceBuffer, argumentPtrs.data(), (uint32)argumentPtrs.size(), pDefaultIncludeHandler.Get(), IID_PPV_ARGS(pCompileResult.GetAddressOf()));

        HRESULT hrCompilation{};
        pCompileResult->GetStatus(&hrCompilation);


        if (SUCCEEDED(hrCompilation))
        {
            ComPtr<IDxcBlob> compiledShaderBlob;
            pCompileResult->GetResult(compiledShaderBlob.GetAddressOf());

            return compiledShaderBlob;
        }
        else {
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> printBlob;
            Microsoft::WRL::ComPtr<IDxcBlobEncoding> printBlob16;

            pCompileResult->GetErrorBuffer(printBlob.GetAddressOf());
            // We can use the library to get our preferred encoding.
            mLibrary->GetBlobAsUtf16(printBlob.Get(), printBlob16.GetAddressOf());
            OutputDebugStringW((LPWSTR)printBlob16->GetBufferPointer());

            ThrowIfFailed(hrCompilation);
            return nullptr;
        }
    }
}