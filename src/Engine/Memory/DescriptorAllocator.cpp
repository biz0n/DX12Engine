#include "DescriptorAllocator.h"

#include <Memory/DescriptorAllocatorPage.h>
#include <Memory/DescriptorAllocation.h>

#include <cassert>
#include <vector>

namespace Engine::Memory
{
	DescriptorAllocator::DescriptorAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, SharedPtr<DescriptorAllocatorPool> newPool, uint32 numDescriptorsPerPage)
	{
		mPage = MakeShared<DescriptorAllocatorPage>(device, type, numDescriptorsPerPage);
		NewPool = newPool;
	}

	DescriptorAllocation DescriptorAllocator::Allocate(uint32 count)
	{
		return mPage->Allocate(count);
	}

	void DescriptorAllocator::ReleaseStaleDescriptors(uint64 frameNumber)
	{
		mPage->ReleaseStaleDescriptors(frameNumber);
		mPage->SetCurrentFrame(frameNumber);
	}
} // namespace Engine::Memory