#include <Exceptions.h>
#include <Window.h>
#include <GameV2.h>

#include <d3d12.h>
#include <dxgi1_6.h>
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

	CoInitialize(nullptr);
	char buffer[MAX_PATH];
	GetCurrentDirectoryA(256, buffer);

	int retCode = 0;

	Engine::Window window(800, 600, TEXT("Window"));

	Engine::GameV2 game;
	retCode = window.Run(&game);

	/*Engine::App app;

	SharedPtr<Engine::Game> game = MakeShared<Engine::Game>(&app);
	try
	{
		retCode = app.Run(game);
	}
	catch (Engine::DxException e)
	{
		MessageBox(nullptr, e.ToString().c_str(), TEXT("HR Failed"), MB_OK);
		retCode = e.ErrorCode;
	}*/

	atexit(&ReportLiveObjects);
	return retCode;
}
