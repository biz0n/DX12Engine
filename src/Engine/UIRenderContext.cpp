#include "UIRenderContext.h"

#include <View.h>
#include <Scene/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>

#include <cassert>

namespace Engine
{
    UIRenderContext::UIRenderContext(View view, ComPtr<ID3D12Device> device, uint32 numFramesInFlight, DXGI_FORMAT rtvFormat)
        : mDevice(device), mNumFramesInFlight(numFramesInFlight), mCurrentFrameIndex(0)
    {
        mDescriptorAllocator = MakeUnique<ImGuiDescriptorAllocator>(device, numFramesInFlight, NumDescriptors);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(view.WindowHandle);

        ImGui_ImplDX12_Init(
            device.Get(),
            numFramesInFlight,
            rtvFormat,
            mDescriptorAllocator->GetD3D12DescriptorHeap().Get(),
            mDescriptorAllocator->GetD3D12DescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
            mDescriptorAllocator->GetD3D12DescriptorHeap()->GetGPUDescriptorHandleForHeapStart());
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

        mDescriptorAllocator->Reset(mCurrentFrameIndex);
    }

    void UIRenderContext::Draw(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        mDescriptorAllocator->CopyStagedDescriptors(mCurrentFrameIndex);

        commandList->SetDescriptorHeaps(1, mDescriptorAllocator->GetD3D12DescriptorHeap().GetAddressOf());
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mNumFramesInFlight;
    }

    void UIRenderContext::Resize(uint32 width, uint32 height)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();

        ImGui_ImplDX12_CreateDeviceObjects();
    }

    ImTextureID UIRenderContext::GetTextureId(D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle = mDescriptorAllocator->StageDescriptor(mCurrentFrameIndex, descriptor);

        ImTextureID textureId = (ImTextureID)handle.ptr;

        return textureId;
    }

    ImGuiDescriptorAllocator::ImGuiDescriptorAllocator(ComPtr<ID3D12Device> device, uint32 framesInFlight, uint32 numDescriptorsPerFrame)
        : mDevice(device), mNumDescriptors(numDescriptorsPerFrame)
    {
        auto realNumDescriptors = ImguiReservedDescriptors + framesInFlight * numDescriptorsPerFrame;
        mHandlersCache = MakeUnique<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>>();
        mHandlersCache->resize(realNumDescriptors);

        mOffsets.resize(framesInFlight);

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = realNumDescriptors;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mSRVHeap)));

        mIncrementalDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE ImGuiDescriptorAllocator::StageDescriptor(uint32 frameIndex, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
    {
        uint32 frameOffset = mOffsets[frameIndex];

        assert(frameOffset < mNumDescriptors);

        auto heapOffset = static_cast<uint32>(ImguiReservedDescriptors + frameOffset + frameIndex * mNumDescriptors);

        D3D12_GPU_DESCRIPTOR_HANDLE handle{
            mSRVHeap->GetGPUDescriptorHandleForHeapStart().ptr + heapOffset * mIncrementalDescriptorSize};

        mHandlersCache->at(heapOffset) = descriptor;
        ++mOffsets[frameIndex];

        return handle;
    }

    void ImGuiDescriptorAllocator::CopyStagedDescriptors(uint32 frameIndex)
    {
        uint32 numDescriptors = mOffsets[frameIndex];
        uint32 handleOffset = ImguiReservedDescriptors + frameIndex * mNumDescriptors;
        D3D12_CPU_DESCRIPTOR_HANDLE mCurrentCpuHandle{
            mSRVHeap->GetCPUDescriptorHandleForHeapStart().ptr + handleOffset * mIncrementalDescriptorSize};

        D3D12_CPU_DESCRIPTOR_HANDLE *srcDescriptor = mHandlersCache->data() + handleOffset;

        mDevice->CopyDescriptors(1, &mCurrentCpuHandle, &numDescriptors, numDescriptors, srcDescriptor, nullptr, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }

    void ImGuiDescriptorAllocator::Reset(uint32 frameIndex)
    {
        mOffsets[frameIndex] = 0;
    }

} // namespace Engine