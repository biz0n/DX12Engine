#include "RootSignatureBuilder.h"

#include <HAL/RootSignature.h>

namespace Engine::Render
{
    RootSignatureBuilder::RootSignatureBuilder()
    {

    }

    RootSignatureBuilder::~RootSignatureBuilder() = default;



    RootSignatureBuilder& RootSignatureBuilder::AddCBVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility)
    {
        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsConstantBufferView(registerIndex, registerSpace, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE,  visibility);

        mParameters.push_back({parameter, std::nullopt});

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSRVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility)
    {
        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsShaderResourceView(registerIndex, registerSpace, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, visibility);

        mParameters.push_back({parameter, std::nullopt});

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUAVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility)
    {
        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsUnorderedAccessView(registerIndex, registerSpace, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, visibility);

        mParameters.push_back({parameter, std::nullopt});

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSRVDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility, uint32 numDescriptors)
    {
        CD3DX12_DESCRIPTOR_RANGE1 table;
        table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, numDescriptors, registerIndex, registerSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsDescriptorTable(1, &table, visibility);
        mParameters.push_back({parameter, table});

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddUAVDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility, uint32 numDescriptors)
    {
        CD3DX12_DESCRIPTOR_RANGE1 table;
        table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, numDescriptors, registerIndex, registerSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsDescriptorTable(1, &table, visibility);
        mParameters.push_back({parameter, table});

        return *this;
    }

    RootSignatureBuilder& RootSignatureBuilder::AddSamplerDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility, uint32 numDescriptors)
    {
        CD3DX12_DESCRIPTOR_RANGE1 table;
        table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, numDescriptors, registerIndex, registerSpace, D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

        CD3DX12_ROOT_PARAMETER1 parameter;
        parameter.InitAsDescriptorTable(1, &table, visibility);
        mParameters.push_back({parameter, table});

        return *this;
    }

    UniquePtr<HAL::RootSignature> RootSignatureBuilder::Build(ComPtr<ID3D12Device2> device)
    {
        const D3D12_STATIC_SAMPLER_DESC sampler = CD3DX12_STATIC_SAMPLER_DESC(
            0,                               // shaderRegister
            D3D12_FILTER_ANISOTROPIC,        // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP);

        const CD3DX12_STATIC_SAMPLER_DESC shadow(
            1, // shaderRegister
            D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
            0.0f,                               // mipLODBias
            16,                                 // maxAnisotropy
            D3D12_COMPARISON_FUNC_LESS_EQUAL,
            D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

        std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
        parameters.reserve(mParameters.size());

        for (auto &p : mParameters) 
        {
            auto& parameter = std::get<0>(p);
            if (parameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
            {
                auto& range = std::get<1>(p);
                parameter.DescriptorTable.NumDescriptorRanges = 1;
                parameter.DescriptorTable.pDescriptorRanges = &range.value();
            }

            parameters.push_back(parameter);
        }

        const D3D12_STATIC_SAMPLER_DESC samplers[2] = {sampler, shadow};
        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.Init_1_1(static_cast<uint32>(parameters.size()), parameters.data(), std::size(samplers), samplers,
                             D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | 
                             D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | 
                             D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED);

        

        mParameters.clear();

        return MakeUnique<HAL::RootSignature>(device, &rootSigDesc);
    }
}