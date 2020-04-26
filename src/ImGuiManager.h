#pragma once

#include "Types.h"
#include "Exceptions.h"

#include <d3d12.h>

class ImGuiManager
{
private:
    ComPtr<ID3D12DescriptorHeap> mSRVHeap;
public:
    ImGuiManager();
    ~ImGuiManager();

    void Initialize(ComPtr<ID3D12Device> device, uint32 numFramesInFlight, DXGI_FORMAT rtvFormat);
    void Deinitialize();

    void BeginFrame();
    void Draw(ComPtr<ID3D12GraphicsCommandList> commandList);
};
