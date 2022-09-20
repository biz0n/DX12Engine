#include "Exceptions.h"
#include <comdef.h>

namespace Engine
{
    DxException::DxException(HRESULT hr, const std::string&functionName, const std::string&filename, int lineNumber) : ErrorCode(hr),
                                                                                                                 FunctionName(functionName),
                                                                                                                 Filename(filename),
                                                                                                                 LineNumber(lineNumber)
    {
        msg = ToString();
    }

    std::string DxException::ToString() const
    {
        // Get the string description of the error code.
        _com_error err(ErrorCode);
        std::string msg = err.ErrorMessage();
        CHAR buffer[5];
        _itot_s(LineNumber, buffer, 10);
        std::string lineNumberStr = buffer;

        return FunctionName + ("\nFile: ") + Filename + ("\nLine: ") + lineNumberStr + ("\nError: ") + msg;
    }

    std::string HrCodeToErrorString(HRESULT errorCode)
    {
        _com_error err(errorCode);
        std::string msg = err.ErrorMessage();
        return msg;
    }

} // namespace Engine