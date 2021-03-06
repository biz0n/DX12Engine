#pragma once

#include <Types.h>
#include <d3d12.h>

#include <d3dx12.h>

namespace Engine::Render
{
    class RootSignature
    {
    public:
        RootSignature(ComPtr<ID3D12Device> device, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC *description);
        ~RootSignature();

        ComPtr<ID3D12RootSignature> GetD3D12RootSignature() const { return mRootSignature; }

        uint32 GetRootParametersCount() const { return mNumRootParameters; }

        uint32 GetDescriptorsBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

        uint32 GetNumDescriptorsPerTable(uint32 index) const;

        static const uint32 MaxDescriptorTables = 32;

    private:
        ComPtr<ID3D12RootSignature> mRootSignature;
        uint32 mNumRootParameters;
        uint32 mNumDescriptorsPerTable[MaxDescriptorTables];
        uint32 mDescriptorTableBitMask;
        uint32 mSamplerTableBitMask;
    };

} // namespace Engine::Render