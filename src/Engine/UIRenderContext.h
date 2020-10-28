#pragma once

#include <Types.h>
#include <Exceptions.h>
#include <Scene/SceneForwards.h>

#include <imgui/imgui.h>

#include <d3d12.h>
#include <vector>

namespace Engine
{
    class ImGuiDescriptorAllocator;
    struct View;

    class UIRenderContext
    {
    private:
        ComPtr<ID3D12Device> mDevice;
        UniquePtr<ImGuiDescriptorAllocator> mDescriptorAllocator;
        uint32 mCurrentFrameIndex;
        uint32 mNumFramesInFlight;

        const uint32 NumDescriptors = 32;

        uint32 mWidth;
        uint32 mHeight;

    public:
        UIRenderContext(View view, ComPtr<ID3D12Device> device, uint32 numFramesInFlight, DXGI_FORMAT rtvFormat);
        ~UIRenderContext();

        void BeginFrame();
        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList);

        void Resize(uint32 width, uint32 height);

        ImTextureID GetTextureId(D3D12_CPU_DESCRIPTOR_HANDLE descriptor);
    };

    class ImGuiDescriptorAllocator
    {
    public:
        ImGuiDescriptorAllocator(ComPtr<ID3D12Device> device, uint32 framesInFlight, uint32 numDescriptors = 32);
        ~ImGuiDescriptorAllocator() = default;

        D3D12_GPU_DESCRIPTOR_HANDLE StageDescriptor(uint32 frameIndex, D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        void CopyStagedDescriptors(uint32 frameIndex);

        void Reset(uint32 frameIndex);

        ComPtr<ID3D12DescriptorHeap> GetD3D12DescriptorHeap() const { return mSRVHeap; }

    private:
        const uint32 ImguiReservedDescriptors = 1;

        ComPtr<ID3D12Device> mDevice;
        ComPtr<ID3D12DescriptorHeap> mSRVHeap;
        uint32 mIncrementalDescriptorSize;

        uint32 mNumDescriptors;
        UniquePtr<std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>> mHandlersCache;
        std::vector<uint32> mOffsets;
    };

} // namespace Engine