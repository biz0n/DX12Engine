#include "RootSignature.h"

#include "Exceptions.h"

namespace Engine
{

    RootSignature::RootSignature(ComPtr<ID3D12Device> device, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC *description)
        : mDescriptorTableBitMask(0), mSamplerTableBitMask(0), mNumDescriptorsPerTable{0}
    {
        auto desc = &description->Desc_1_1;
        mNumRootParameters = desc->NumParameters;
        for (uint32 i = 0; i < desc->NumParameters; ++i)
        {
            const auto &rootParameter = desc->pParameters[i];

            if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                uint32 numDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;

                if (numDescriptorRanges > 0)
                {
                    switch (rootParameter.DescriptorTable.pDescriptorRanges[0].RangeType)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                        mDescriptorTableBitMask |= (1 << i);
                        break;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                        mSamplerTableBitMask |= (1 << i);
                        break;
                    }
                }

                for (uint32 j = 0; j < numDescriptorRanges; ++j)
                {
                    mNumDescriptorsPerTable[i] += rootParameter.DescriptorTable.pDescriptorRanges[j].NumDescriptors;
                }
            }
        }

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(description, D3D_ROOT_SIGNATURE_VERSION_1_1,
                                                           serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
        ThrowIfFailed(hr);

        ThrowIfFailed(device->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&mRootSignature)));
    }

    uint32 RootSignature::GetDescriptorsBitMask(D3D12_DESCRIPTOR_HEAP_TYPE type) const
    {
        switch (type)
        {
        case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            return mDescriptorTableBitMask;
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
            return mSamplerTableBitMask;
        }
        return 0;
    }

    uint32 RootSignature::GetNumDescriptorsPerTable(uint32 index) const
    {
        return mNumDescriptorsPerTable[index];
    }

} // namespace Engine