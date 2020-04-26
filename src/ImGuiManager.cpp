#include "ImGuiManager.h"
#include "Libraries/imgui/imgui.h"
#include "Libraries/imgui/imgui_impl_dx12.h"
#include "Libraries/imgui/imgui_impl_win32.h"

ImGuiManager::ImGuiManager()
{
}

ImGuiManager::~ImGuiManager()
{
}

void ImGuiManager::Initialize(ComPtr<ID3D12Device> device, uint32 numFramesInFlight, DXGI_FORMAT rtvFormat)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSRVHeap)));

    ImGui_ImplDX12_Init(
        device.Get(),
        numFramesInFlight,
        rtvFormat,
        mSRVHeap.Get(),
        mSRVHeap->GetCPUDescriptorHandleForHeapStart(),
        mSRVHeap->GetGPUDescriptorHandleForHeapStart());
}

void ImGuiManager::Deinitialize()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::BeginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
{
    commandList->SetDescriptorHeaps(1, mSRVHeap.GetAddressOf());
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}