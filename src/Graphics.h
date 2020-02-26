#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "Types.h"

class Graphics
{
public:
    Graphics();
    ~Graphics();

    Graphics(const Graphics &) = delete;
    Graphics &operator=(const Graphics &) = delete;

    inline Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const { return device; }
    inline Microsoft::WRL::ComPtr<IDXGIFactory4> GetGIFactory() const { return dxgiFactory; }
    inline bool IsTearingSupported() const { return isTearingSupported; }

private:
    bool CheckTearing();

private:
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;

    bool isTearingSupported;
};