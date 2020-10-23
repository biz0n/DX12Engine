#include <Exceptions.h>
#include <Window.h>
#include <Application.h>

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

	Engine::Application app;
	Engine::Window window(800, 600, TEXT("Window"));

	int retCode = window.Run(&app);

	atexit(&ReportLiveObjects);
	return retCode;
}
