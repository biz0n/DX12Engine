#include "DescriptorAllocator.h"

#include <Memory/DescriptorAllocatorPage.h>
#include <Memory/DescriptorAllocation.h>

namespace Engine
{
	DescriptorAllocator::DescriptorAllocator(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPerPage)
	{
		mPage = MakeShared<DescriptorAllocatorPage>(device, type, numDescriptorsPerPage);
	}

	DescriptorAllocation DescriptorAllocator::Allocate(uint32 count)
	{
		return mPage->Allocate(count);
	}

	void DescriptorAllocator::ReleaseStaleDescriptors(uint64 frameNumber)
	{
		mPage->ReleaseStaleDescriptors(frameNumber);
	}
} // namespace Engine