#include "Graphics.h"
#include "Exceptions.h"

using Microsoft::WRL::ComPtr;

Graphics::Graphics() : isTearingSupported(false)
{
#if defined(DEBUG) || defined(_DEBUG)
    ComPtr<ID3D12Debug> debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
#endif
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

    HRESULT createDeviceResult = D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&device));

    if (FAILED(createDeviceResult))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

        ThrowIfFailed(D3D12CreateDevice(
            warpAdapter.Get(),
            D3D_FEATURE_LEVEL_11_0,
            IID_PPV_ARGS(&device)));
    }

    isTearingSupported = CheckTearing();
}

Graphics::~Graphics()
{
}

bool Graphics::CheckTearing()
{
    BOOL allowTearing = FALSE;

    ComPtr<IDXGIFactory5> factory5;
    if (SUCCEEDED(dxgiFactory.As(&factory5)))
    {
        ThrowIfFailed(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));
    }

    return allowTearing == TRUE;
}