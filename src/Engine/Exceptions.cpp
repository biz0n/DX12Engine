#include "Exceptions.h"
#include <comdef.h>

DxException::DxException(HRESULT hr, const TString &functionName, const TString &filename, int lineNumber) : ErrorCode(hr),
                                                                                                             FunctionName(functionName),
                                                                                                             Filename(filename),
                                                                                                             LineNumber(lineNumber)
{
}

TString DxException::ToString() const
{
    // Get the string description of the error code.
    _com_error err(ErrorCode);
    TString msg = err.ErrorMessage();
    TCHAR buffer[4];
    TString lineNumberStr = _itot(LineNumber, buffer, 10);

    return FunctionName + TEXT(" failed in ") + Filename + TEXT("; line ") + lineNumberStr + TEXT("; error: ") + msg;
}