#include "ResourceStateTracker.h"

namespace Engine
{

	GlobalResourceStateTracker::GlobalResourceStateTracker()
	{

	}

	void GlobalResourceStateTracker::TrackResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
	{
		if (resource)
		{
			mStates[resource] = state;
		}
	}

	void GlobalResourceStateTracker::UntrackResource(ID3D12Resource* resource)
	{
		if (resource)
		{
			mStates.erase(resource);
		}
	}

	D3D12_RESOURCE_STATES GlobalResourceStateTracker::GetLastState(ID3D12Resource* resource)
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

	void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier)
	{
		if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION)
		{
			const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

			auto resource = transitionBarrier.pResource;
			auto finalState = mGlobalReourceTracker->GetLastState(resource);
			if (finalState != transitionBarrier.StateAfter)
			{
				D3D12_RESOURCE_BARRIER newBarrier = barrier;
				newBarrier.Transition.StateBefore = finalState;
				mBarriers.emplace_back(newBarrier);
				TrackResource(resource, newBarrier.Transition.StateAfter);
			}
		}
		else
		{
			mBarriers.emplace_back(barrier);
		}
	}

	void ResourceStateTracker::FlushBarriers(ComPtr<ID3D12GraphicsCommandList> commandList)
	{
		uint32 numBarriers = static_cast<uint32>(mBarriers.size());
		if (numBarriers > 0)
		{
			commandList->ResourceBarrier(numBarriers, mBarriers.data());
			mBarriers.clear();
		}
	}

	void ResourceStateTracker::TrackResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES state)
	{
		mGlobalReourceTracker->TrackResource(resource, state);
	}

	void ResourceStateTracker::UntrackResource(ID3D12Resource* resource)
	{
		mGlobalReourceTracker->UntrackResource(resource);
	}

}