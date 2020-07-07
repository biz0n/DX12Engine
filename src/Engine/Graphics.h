#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Types.h>

namespace Engine
{
    class Graphics
    {
    public:
        Graphics();
        ~Graphics();

        Graphics(const Graphics &) = delete;
        Graphics &operator=(const Graphics &) = delete;

        inline ComPtr<ID3D12Device2> GetDevice() const { return device; }
        inline ComPtr<IDXGIFactory4> GetGIFactory() const { return dxgiFactory; }

    private:
        Microsoft::WRL::ComPtr<ID3D12Device2> device;
        Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
    };

} // namespace Engine