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
        .AddCBVParameter(0, 10)
        .AddCBVParameter(1, 10)
        .AddCBVParameter(2, 10)
        .AddSRVDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddSRVDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddSRVDescriptorTableParameter(0, 12, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddSRVDescriptorTableParameter(0, 13, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddSRVDescriptorTableParameter(0, 14, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 12, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 13, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 14, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddUAVDescriptorTableParameter(0, 15, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize)
        .AddSamplerDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize);
    }
} // namespace Engine::Render