#pragma once

#include <Windows.h>
#include <Types.h>

#include <sstream>
#include <cassert>

#include <windows.h>

namespace Engine
{
    class DxException : public std::exception
    {
    private:
        TString msg;
    public:
        DxException() = default;
        DxException(HRESULT hr, const TString &functionName, const TString &filename, int lineNumber);
   
        TString ToString() const;

        virtual const char* what() const throw ()
        {
            return msg.c_str();;
        }

        HRESULT ErrorCode = S_OK;
        TString FunctionName;
        TString Filename;
        int LineNumber = -1;
    };

    template< typename... Args >
    inline void print_assertion(Args&&... args)
    {
        std::stringstream ss;
        ss.precision(10);
        ss << std::endl;
        (ss << ... << args) << std::endl;

        OutputDebugString(ss.str().c_str());
        abort();
    }

    TString HrCodeToErrorString(HRESULT errorCode);


} // namespace Engine

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                                         \
    do {                                                                         \
        HRESULT hr__ = (x);                                                      \
        if (FAILED(hr__))                                                        \
        {                                                                        \
            Engine::print_assertion(                                             \
                "Result: ",                                                      \
                hr__,                                                            \
                " Function: ",                                                   \
                 #x,                                                             \
                " in File: ",                                                    \
                __FILE__,                                                        \
                " in Line: ",                                                    \
                __LINE__,                                                        \
                " Error Message: ",                                              \
                Engine::HrCodeToErrorString(hr__));                              \
            /*throw Engine::DxException(hr__, TEXT(#x), TEXT(__FILE__), __LINE__); */ \
        }                                                                        \
    }while (0)
#endif


#ifdef assert_format
#undef assert_format
#endif
#define assert_format(EXPRESSION, ... ) ((EXPRESSION) ? (void)0 : print_assertion(\
        "Error: ", \
        #EXPRESSION, \
        " in File: ", \
        __FILE__, \
        " in Line: ", \
        __LINE__, \
        " \n",\
        __VA_ARGS__))
