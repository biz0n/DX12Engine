#include "ResourceStateTracker.h"

namespace Engine::Memory
{

    GlobalResourceStateTracker::GlobalResourceStateTracker()
    {
    }

    void GlobalResourceStateTracker::TrackResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES state)
    {
        if (resource)
        {
            mStates[resource] = state;
        }
    }

    void GlobalResourceStateTracker::UntrackResource(ID3D12Resource *resource)
    {
        if (resource)
        {
            mStates.erase(resource);
        }
    }

    D3D12_RESOURCE_STATES GlobalResourceStateTracker::GetLastState(ID3D12Resource *resource)
    {
        auto iter = mStates.find(resource);
        if (iter != mStates.end())
        {
            return iter->second;
        }

        assert(false);
        return D3D12_RESOURCE_STATE_COMMON;
    }

    ResourceStateTracker::ResourceStateTracker(SharedPtr<GlobalResourceStateTracker> globalResourceTracker) : mGlobalReourceTracker(globalResourceTracker)
    {
    }

    void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER &barrier)
    {
        if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
        {
            const D3D12_RESOURCE_TRANSITION_BARRIER &transitionBarrier = barrier.Transition;

            auto resource = transitionBarrier.pResource;
            auto iter = mFinalStates.find(resource);
            if (iter != mFinalStates.end())
            {
                auto finalState = iter->second;
                if (finalState != transitionBarrier.StateAfter)
                {
                    D3D12_RESOURCE_BARRIER newBarrier = barrier;
                    newBarrier.Transition.StateBefore = finalState;
                    mBarriers.emplace_back(newBarrier);
                }
            }
            else
            {
                mPendingBarriers.emplace_back(barrier);
            }

            mFinalStates[transitionBarrier.pResource] = transitionBarrier.StateAfter;
        }
        else
        {
            mBarriers.emplace_back(barrier);
        }
    }

    void ResourceStateTracker::FlushBarriers(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        FlushBarriers(commandList.Get());
    }

    void ResourceStateTracker::FlushBarriers(ID3D12GraphicsCommandList* commandList)
    {
        uint32 numBarriers = static_cast<uint32>(mBarriers.size());
        if (numBarriers > 0)
        {
            commandList->ResourceBarrier(numBarriers, mBarriers.data());
            mBarriers.clear();
        }
    }

    uint32 ResourceStateTracker::FlushPendingBarriers(ComPtr<ID3D12GraphicsCommandList> commandList)
    {
        std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;
        resourceBarriers.reserve(mPendingBarriers.size());

        for (auto &barrier : mPendingBarriers)
        {
            auto *resource = barrier.Transition.pResource;
            auto &stateAfter = barrier.Transition.StateAfter;

            auto globalState = mGlobalReourceTracker->GetLastState(resource);

            if (stateAfter != globalState)
            {
                D3D12_RESOURCE_BARRIER newBarrier = barrier;
                newBarrier.Transition.StateBefore = globalState;
                resourceBarriers.emplace_back(newBarrier);
            }
        }

        uint32 numBarriers = static_cast<uint32>(resourceBarriers.size());
        if (numBarriers > 0)
        {
            commandList->ResourceBarrier(numBarriers, resourceBarriers.data());
        }

        mPendingBarriers.clear();

        return numBarriers;
    }

    void ResourceStateTracker::CommitFinalResourceStates()
    {
        for (auto &iter : mFinalStates)
        {
            mGlobalReourceTracker->TrackResource(iter.first, iter.second);
        }

        mFinalStates.clear();
    }

    void ResourceStateTracker::TrackResource(ID3D12Resource *resource, D3D12_RESOURCE_STATES state)
    {
        mGlobalReourceTracker->TrackResource(resource, state);
    }

    void ResourceStateTracker::UntrackResource(ID3D12Resource *resource)
    {
        mGlobalReourceTracker->UntrackResource(resource);
    }

} // namespace Engine::Render