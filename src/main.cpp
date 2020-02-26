#include "App.h"
#include "Exceptions.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	App app;
	try
	{
		return app.Run();
	}
	catch (DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), TEXT("HR Failed"), MB_OK);
		return e.ErrorCode;
	}
}
