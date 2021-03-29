#include "RootSignatureProvider.h"

#include <Render/RootSignatureBuilder.h>
#include <StringUtils.h>

namespace Engine::Render
{

    RootSignatureProvider::RootSignatureProvider(ComPtr<ID3D12Device2> device) : mDevice{device}
    {
    }

    RootSignatureProvider::~RootSignatureProvider() = default;

    void RootSignatureProvider::BuildRootSignature(const Name &name, RootSignatureBuilder &builder)
    {
        if (mRootSignatureMap.contains(name))
        {
            return;
        }

        AddDefaultRegisters(builder);
        auto rs = builder.Build(mDevice);
        rs->GetD3D12RootSignature()->SetName(StringToWString(name.string()).c_str());
        mRootSignatureMap.emplace(name, std::move(rs));
    }

    RootSignature *RootSignatureProvider::GetRootSignature(const Name &name)
    {
        return mRootSignatureMap[name].get();
    }

    void RootSignatureProvider::AddDefaultRegisters(RootSignatureBuilder& builder)
    {
        builder
        .AddCBVParameter(0, 10) // Pass constant buffer
        .AddCBVParameter(1, 10) // Frame constant buffer
        .AddCBVParameter(2, 10) // Global constant buffer
        .AddSRVDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Texture2D
        .AddSRVDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Texture2D<uint4>
        .AddSRVDescriptorTableParameter(0, 12, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Texture3D
        .AddSRVDescriptorTableParameter(0, 13, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Texture3d<uint4>
        .AddSRVDescriptorTableParameter(0, 14, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Texture2DArray
        .AddSRVDescriptorTableParameter(0, 15, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // TextureCube
        .AddSRVDescriptorTableParameter(0, 16, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // TextureCubeArray
        .AddUAVDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture2D<float4> 
        .AddUAVDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture2D<uint4>
        .AddUAVDescriptorTableParameter(0, 12, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture2D<uint>
        .AddUAVDescriptorTableParameter(0, 13, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture3D<float4>
        .AddUAVDescriptorTableParameter(0, 14, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture3D<uint4>
        .AddUAVDescriptorTableParameter(0, 15, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // RWTexture2DArray<float4>
        .AddSamplerDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Sampler[]
        .AddSamplerDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize); // SamplerComparisonState[]
    }
} // namespace Engine::Render