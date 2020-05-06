#include "App.h"
#include "Exceptions.h"
#include "Game.h"

#include <dxgidebug.h>

void ReportLiveObjects()
{
	IDXGIDebug1 *dxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));

	dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_IGNORE_INTERNAL);
	dxgiDebug->Release();

	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
}

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	char buffer[MAX_PATH];
	GetCurrentDirectoryA(256, buffer);

	int retCode = 0;

	Engine::App app;

	SharedPtr<Engine::Game> game = MakeShared<Engine::Game>(&app);
	try
	{
		retCode = app.Run(game);
	}
	catch (Engine::DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), TEXT("HR Failed"), MB_OK);
		retCode = e.ErrorCode;
	}

	atexit(&ReportLiveObjects);
	return retCode;
}