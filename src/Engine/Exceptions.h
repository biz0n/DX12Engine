#pragma once

#include <Windows.h>
#include <Types.h>

namespace Engine
{
    class DxException
    {
    public:
        DxException() = default;
        DxException(HRESULT hr, const TString &functionName, const TString &filename, int lineNumber);

        TString ToString() const;

        HRESULT ErrorCode = S_OK;
        TString FunctionName;
        TString Filename;
        int LineNumber = -1;
    };

} // namespace Engine

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                         \
    {                                                                            \
        HRESULT hr__ = (x);                                                      \
        if (FAILED(hr__))                                                        \
        {                                                                        \
            throw Engine::DxException(hr__, TEXT(#x), TEXT(__FILE__), __LINE__); \
        }                                                                        \
    }
#endif