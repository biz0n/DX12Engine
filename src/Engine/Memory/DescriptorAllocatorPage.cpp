#include "DescriptorAllocatorPage.h"

#include <Exceptions.h>
#include <Memory/DescriptorAllocation.h>

#include <d3dx12.h>

namespace Engine
{
	DescriptorAllocatorPage::DescriptorAllocatorPage(ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptorsPerPage)
		: mHeapType(type), mNumDescriptorsPerPage(numDescriptorsPerPage)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = mNumDescriptorsPerPage;
		heapDesc.Type = mHeapType;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mHeap)));

		mDescriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(mHeapType);
		mBaseDescriptor = mHeap->GetCPUDescriptorHandleForHeapStart();

		mFreeSize = numDescriptorsPerPage;
		AddNewBlock(0, numDescriptorsPerPage);
	}

	DescriptorAllocation DescriptorAllocatorPage::Allocate(uint32 count)
	{
		if (mFreeSize < count)
		{
			return DescriptorAllocation();
		}

		auto smallestBlockIt = mFreeBlockBySizeMap.lower_bound(count);
		if (smallestBlockIt == mFreeBlockBySizeMap.end())
		{
			return DescriptorAllocation();
		}
		auto smallestBlock = smallestBlockIt->second;
		auto offset = smallestBlock->first;

		auto newOffset = offset + count;
		auto newSize = smallestBlock->second.BlockSize - count;

		mFreeBlockBySizeMap.erase(smallestBlockIt);
		mFreeBlockByOffsetMap.erase(smallestBlock);

		if (newSize > 0)
		{
			AddNewBlock(newOffset, newSize);
		}

		mFreeSize -= count;

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle(mBaseDescriptor, static_cast<int32>(offset), mDescriptorHandleIncrementSize);

		return DescriptorAllocation(handle, count, mDescriptorHandleIncrementSize, shared_from_this());
	}

	void DescriptorAllocatorPage::Free(DescriptorAllocation &&descriptorHandle, uint64_t frameNumber)
	{
		auto offset = CalculateOffset(descriptorHandle.GetDescriptor());
		auto count = descriptorHandle.GetNumDescsriptors();

		mStaleDescriptors.emplace(offset, count, frameNumber);
	}

	void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64 frameNumber)
	{
		while (!mStaleDescriptors.empty() && mStaleDescriptors.front().FrameNumber <= frameNumber)
		{
			auto &staleDescriptor = mStaleDescriptors.front();

			auto offset = staleDescriptor.Offset;
			auto numDescriptors = staleDescriptor.Count;

			FreeBlock(offset, numDescriptors);

			mStaleDescriptors.pop();
		}
	}

	void DescriptorAllocatorPage::AddNewBlock(Index offset, Size size)
	{
		auto newBlockIt = mFreeBlockByOffsetMap.emplace(offset, size);
		auto orderIt = mFreeBlockBySizeMap.emplace(size, newBlockIt.first);
		newBlockIt.first->second.OrderBySizeIt = orderIt;
	}

	void DescriptorAllocatorPage::FreeBlock(Index offset, Size size)
	{
		auto nextBlockIt = mFreeBlockByOffsetMap.upper_bound(offset);
		auto prevBlockIt = nextBlockIt;

		if (prevBlockIt != mFreeBlockByOffsetMap.begin())
		{
			prevBlockIt--;
		}
		else
		{
			prevBlockIt = mFreeBlockByOffsetMap.end();
		}

		if (prevBlockIt != mFreeBlockByOffsetMap.end() && offset == prevBlockIt->first + prevBlockIt->second.BlockSize)
		{
			// PrevBlock.Offset           Offset
			// |                          |
			// |<-----PrevBlock.Size----->|<------Size-------->|
			//
			size += prevBlockIt->second.BlockSize;
			offset = prevBlockIt->first;

			mFreeBlockBySizeMap.erase(prevBlockIt->second.OrderBySizeIt);
			mFreeBlockByOffsetMap.erase(prevBlockIt);
		}
		if (nextBlockIt != mFreeBlockByOffsetMap.end() && offset + size == nextBlockIt->first)
		{
			// Offset               NextBlock.Offset
			// |                    |
			// |<------Size-------->|<-----NextBlock.Size----->|
			//
			size += nextBlockIt->second.BlockSize;
			mFreeBlockBySizeMap.erase(nextBlockIt->second.OrderBySizeIt);
			mFreeBlockByOffsetMap.erase(nextBlockIt);
		}

		AddNewBlock(offset, size);

		mFreeSize += size;
	}

	Index DescriptorAllocatorPage::CalculateOffset(const D3D12_CPU_DESCRIPTOR_HANDLE &handle)
	{
		return static_cast<Index>((handle.ptr - mBaseDescriptor.ptr) / mDescriptorHandleIncrementSize);
	}

} // namespace Engine