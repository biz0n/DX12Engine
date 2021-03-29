#pragma once

#include <Types.h>
#include <Exceptions.h>
#include <Scene/SceneForwards.h>
#include <Memory/DescriptorAllocatorPool.h>

#include <imgui/imgui.h>

#include <d3d12.h>
#include <vector>

namespace Engine
{
    struct View;
}

namespace Engine::Render
{
    class UIRenderContext
    {
    private:
        ComPtr<ID3D12Device> mDevice;

        Memory::DescriptorAllocatorPool* mDescriptorAllocatorPool;
        Memory::DescriptorAllocation mFontDescriptorAllocation;

    public:
        UIRenderContext(
            View view,
            ComPtr<ID3D12Device> device, 
            Memory::DescriptorAllocatorPool* descriptorAllocatorPool,
            uint32 numFramesInFlight, 
            DXGI_FORMAT rtvFormat);
        ~UIRenderContext();

        void BeginFrame();
        void Draw(ComPtr<ID3D12GraphicsCommandList> commandList);

        void Resize(uint32 width, uint32 height);
    };
} // namespace Engine::Render