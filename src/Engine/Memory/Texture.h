#pragma once

#include <Types.h>
#include <Exceptions.h>
#include <Memory/DescriptorAllocatorPool.h>
#include <Memory/ResourceAllocator.h>
#include <Memory/ResourceCopyManager.h>
#include <Memory/Resource.h>
#include <Render/RenderForwards.h>
#include <vector>
#include <d3d12.h>
#include <d3dx12.h>


namespace Engine::Memory
{
    class Texture : public Resource
    {
    public:
        Texture(ID3D12Device *device,
                ResourceAllocator *resourceAllocator,
                Engine::Memory::DescriptorAllocatorPool *descriptorAllocator,
                Engine::Render::GlobalResourceStateTracker* stateTracker,
                D3D12_RESOURCE_DESC desc,
                const D3D12_CLEAR_VALUE *clearValue,
                D3D12_RESOURCE_STATES state);

        Texture(ComPtr<ID3D12Resource> resource,
                Engine::Memory::DescriptorAllocatorPool *descriptorAllocator,
                Engine::Render::GlobalResourceStateTracker* stateTracker,
                D3D12_RESOURCE_STATES state);

        ~Texture() override;

        const Engine::Memory::DescriptorAllocation &GetCBDescriptor() const;

        const Engine::Memory::DescriptorAllocation &GetSRDescriptor() const;

        const Engine::Memory::DescriptorAllocation &GetCubeSRDescriptor() const;

        const Engine::Memory::DescriptorAllocation &GetUADescriptor(uint32 mipSlice = 0) const;

        const Engine::Memory::DescriptorAllocation &GetRTDescriptor(uint32 mipSlice = 0) const;

        const Engine::Memory::DescriptorAllocation &GetDSDescriptor(uint32 mipSlice = 0) const;



        Size GetAlignment() const { return mAlignment; }

        Size GetSubresourcesCount() const override ;

        CopyCommandFunction GetCopyCommandFunction() const override;

    private:
        Engine::Memory::DescriptorAllocatorPool *mDescriptorAllocator;
        Engine::Render::GlobalResourceStateTracker* mStateTracker;
        Size mAlignment;
        Size mSize;

        mutable Engine::Memory::DescriptorAllocation mCBDescriptor{};
        mutable Engine::Memory::DescriptorAllocation mSRDescriptor{};
        mutable Engine::Memory::DescriptorAllocation mCubeSRDescriptor{};

        mutable std::vector<Engine::Memory::DescriptorAllocation> mUADescriptor;
        mutable std::vector<Engine::Memory::DescriptorAllocation> mRTDescriptor;
        mutable std::vector<Engine::Memory::DescriptorAllocation> mDSDescriptor;
    };
}