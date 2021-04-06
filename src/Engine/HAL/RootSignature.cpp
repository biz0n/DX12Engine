#include "RootSignature.h"

#include <Exceptions.h>
#include <Hash.h>

namespace Engine::HAL
{
    RootSignature::RootSignature(ComPtr<ID3D12Device> device, const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC *description)
        : mDescriptorTableBitMask(0), mSamplerTableBitMask(0), mNumDescriptorsPerTable{0}
    {
        auto desc = &description->Desc_1_0;
        mNumRootParameters = desc->NumParameters;
        for (uint32 i = 0; i < desc->NumParameters; ++i)
        {
            const auto &rootParameter = desc->pParameters[i];

            auto type = RegisterType::ShaderResource;
            uint32 shaderRegister = 0;
            uint32 registerSpace = 0;
            if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            {
                type = RegisterType::ConstantBuffer;
                shaderRegister = rootParameter.Constants.ShaderRegister;
                registerSpace = rootParameter.Constants.RegisterSpace;
            }
            else if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
            {
                type = RegisterType::ConstantBuffer;
                shaderRegister = rootParameter.Descriptor.ShaderRegister;
                registerSpace = rootParameter.Descriptor.RegisterSpace;
            }
            else if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
            {
                type = RegisterType::ShaderResource;
                shaderRegister = rootParameter.Descriptor.ShaderRegister;
                registerSpace = rootParameter.Descriptor.RegisterSpace;
            }
            else if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
            {
                type = RegisterType::UnorderedAccess;
                shaderRegister = rootParameter.Descriptor.ShaderRegister;
                registerSpace = rootParameter.Descriptor.RegisterSpace;
            }
            else if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                uint32 numDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;

                if (numDescriptorRanges > 0)
                {
                    shaderRegister = rootParameter.DescriptorTable.pDescriptorRanges[0].BaseShaderRegister;
                    registerSpace = rootParameter.DescriptorTable.pDescriptorRanges[0].RegisterSpace;

                    switch (rootParameter.DescriptorTable.pDescriptorRanges[0].RangeType)
                    {
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                        type = RegisterType::ShaderResource;
                        mDescriptorTableBitMask |= (1 << i);
                        break;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        type = RegisterType::ConstantBuffer;
                        mDescriptorTableBitMask |= (1 << i);
                        break;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                        type = RegisterType::UnorderedAccess;
                        mDescriptorTableBitMask |= (1 << i);
                        break;
                    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                        type = RegisterType::Sampler;
                        mSamplerTableBitMask |= (1 << i);
                        break;
                    }
                }

                for (uint32 j = 0; j < numDescriptorRanges; ++j)
                {
                    mNumDescriptorsPerTable[i] += rootParameter.DescriptorTable.pDescriptorRanges[j].NumDescriptors;
                }
            }

            const auto key = std::hash_combine(type, shaderRegister, registerSpace);
            mIndexMap[key] = i;
        }

        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        HRESULT hr = D3DX12SerializeVersionedRootSignature(description, D3D_ROOT_SIGNATURE_VERSION_1_0,
                                                           serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());
        ThrowIfFailed(hr);

        ThrowIfFailed(device->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&mRootSignature)));
    }

    RootSignature::~RootSignature() = default;

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

    uint32 RootSignature::GetIndex(RootSignature::RegisterType type, uint32 registerIndex, uint32 registerSpace) const
    {
        const auto key = std::hash_combine(type, registerIndex, registerSpace);
        auto iter = mIndexMap.find(key);
        assert_format(iter != mIndexMap.end(), "Register (%i, %i, %i) not found", static_cast<uint32>(type), registerIndex, registerSpace);

        return iter->second;
    }

} // namespace Engine::Render