#pragma once

#include <Types.h>
#include <d3d12.h>

#include <unordered_map>
#include <tuple>
#include <d3dx12.h>

namespace Engine::Render
{
    class RootSignature
    {
    public:
        enum class RegisterType : uint32
        {
            ConstantBuffer = 0,
            ShaderResource = 1,
            UnorderedAccess = 2,
            Sampler = 3
        };
    public:
        RootSignature(ComPtr<ID3D12Device> device, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC *description);
        ~RootSignature();

        ComPtr<ID3D12RootSignature> GetD3D12RootSignature() const { return mRootSignature; }

        uint32 GetRootParametersCount() const { return mNumRootParameters; }

        uint32 GetDescriptorsBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

        uint32 GetNumDescriptorsPerTable(uint32 index) const;

        uint32 GetIndex(RegisterType type, uint32 registerIndex, uint32 registerSpace) const;

        static const uint32 MaxDescriptorTables = 32;

    private:
        ComPtr<ID3D12RootSignature> mRootSignature;
        uint32 mNumRootParameters;
        uint32 mNumDescriptorsPerTable[MaxDescriptorTables];
        uint32 mDescriptorTableBitMask;
        uint32 mSamplerTableBitMask;
        std::unordered_map<size_t, uint32> mIndexMap;
    };

} // namespace Engine::Render