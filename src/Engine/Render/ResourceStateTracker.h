#pragma once

#include <Types.h>

#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <cassert>

namespace Engine::Render
{
    class GlobalResourceStateTracker
    {
    public:
        GlobalResourceStateTracker();

        void TrackResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES state);

        void UntrackResource(ID3D12Resource *resource);

        D3D12_RESOURCE_STATES GetLastState(ID3D12Resource *resource);

    private:
        std::unordered_map<ID3D12Resource *, D3D12_RESOURCE_STATES> mStates;
    };

    class ResourceStateTracker
    {
    public:
        ResourceStateTracker(SharedPtr<GlobalResourceStateTracker> globalResourceTracker);

        void ResourceBarrier(const D3D12_RESOURCE_BARRIER &barrier);

        void FlushBarriers(ComPtr<ID3D12GraphicsCommandList> commandList);
        void FlushBarriers(ID3D12GraphicsCommandList* commandList);

        uint32 FlushPendingBarriers(ComPtr<ID3D12GraphicsCommandList> commandList);

        void CommitFinalResourceStates();

        void TrackResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES state);

        void UntrackResource(ID3D12Resource *resource);

    private:
        SharedPtr<GlobalResourceStateTracker> mGlobalReourceTracker;

        std::vector<D3D12_RESOURCE_BARRIER> mBarriers;
        std::vector<D3D12_RESOURCE_BARRIER> mPendingBarriers;
        std::unordered_map<ID3D12Resource *, D3D12_RESOURCE_STATES> mFinalStates;
    };

} // namespace Engine::Render