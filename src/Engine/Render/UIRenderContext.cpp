#include "UIRenderContext.h"

#include <View.h>
#include <Memory/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/ImGuizmo.h>

#include <cassert>
#include <d3dx12.h>

namespace Engine::Render
{
    UIRenderContext::UIRenderContext(
        View view, 
        ComPtr<ID3D12Device> device, 
        Memory::DescriptorAllocatorPool* descriptorAllocatorPool,
        uint32 numFramesInFlight, 
        DXGI_FORMAT rtvFormat)
        : mDevice(device), mDescriptorAllocatorPool{descriptorAllocatorPool}
    {
        mFontDescriptorAllocation = descriptorAllocatorPool->AllocateSRDescriptor(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(view.WindowHandle);

        ImGui_ImplDX12_Init(
            device.Get(),
            numFramesInFlight,
            rtvFormat,
            descriptorAllocatorPool->GetCbvSrvUavDescriptorHeap(),
            mFontDescriptorAllocation.GetCPUDescriptor(),
            mFontDescriptorAllocation.GetGPUDescriptor());
    }

    UIRenderContext::~UIRenderContext()
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void UIRenderContext::BeginFrame()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    }

    void UIRenderContext::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        ID3D12DescriptorHeap* heap[1] = {mDescriptorAllocatorPool->GetCbvSrvUavDescriptorHeap()};
        commandList->SetDescriptorHeaps(1, heap);
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
    }

    void UIRenderContext::Resize(uint32 width, uint32 height)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();

        ImGui_ImplDX12_CreateDeviceObjects();
    }
} // namespace Engine::Render