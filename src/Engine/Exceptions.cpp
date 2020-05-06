#include "Exceptions.h"
#include <comdef.h>

namespace Engine
{
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
        _itot_s(LineNumber, buffer, 10);
        TString lineNumberStr = buffer;

        return FunctionName + TEXT(" failed in ") + Filename + TEXT("; line ") + lineNumberStr + TEXT("; error: ") + msg;
    }

} // namespace Engine