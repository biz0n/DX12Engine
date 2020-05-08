#include "DescriptorAllocator.h"



Engine::DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 incrementalDescriptorSize, uint32 numDescriptorsPerPage /*= 256*/)
	: mHeapType(type)
	, mIncrementalDescriptorSize(incrementalDescriptorSize)
	, mNumDescriptorsPerPage(numDescriptorsPerPage)
	, mCurrentDescriptorOffset(0)
	, mCurrentHandle{0}
{

}

Engine::DescriptorAllocation Engine::DescriptorAllocator::Allocate(ComPtr<ID3D12Device> device, uint32 count /*= 1*/)
{
	assert(count <= mNumDescriptorsPerPage);

	if (mCurrentHeap == nullptr || (mCurrentDescriptorOffset + count) > mNumDescriptorsPerPage)
	{
		if (mCurrentHeap != nullptr)
		{
			mHeapsPool.emplace_back(mCurrentHeap);
		}
		mCurrentHeap = CreateDescriptorHeap(device);
		mCurrentHandle = mCurrentHeap->GetCPUDescriptorHandleForHeapStart();
		mCurrentDescriptorOffset = 0;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE descriptor{ mCurrentHandle.ptr + (mCurrentDescriptorOffset * mIncrementalDescriptorSize) };
	DescriptorAllocation allocation(descriptor, count, mIncrementalDescriptorSize);

	mCurrentDescriptorOffset += count;

	return allocation;
}

ComPtr<ID3D12DescriptorHeap> Engine::DescriptorAllocator::CreateDescriptorHeap(ComPtr<ID3D12Device> device)
{
	ComPtr<ID3D12DescriptorHeap> heap;
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = mNumDescriptorsPerPage;
	heapDesc.Type = mHeapType;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	return heap;
}
