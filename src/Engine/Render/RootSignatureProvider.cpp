#include "RootSignatureProvider.h"

#include <Render/RootSignatureBuilder.h>
#include <HAL/RootSignature.h>
#include <HAL/DirectXExtensions.h>

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
        HAL::SetResourceName(rs->GetD3D12RootSignature(), name.string());
        mRootSignatureMap.emplace(name, std::move(rs));
    }

    HAL::RootSignature *RootSignatureProvider::GetRootSignature(const Name &name)
    {
        return mRootSignatureMap[name].get();
    }

    void RootSignatureProvider::AddDefaultRegisters(RootSignatureBuilder& builder)
    {
        builder
        .AddCBVParameter(0, 10) // Pass constant buffer
        .AddCBVParameter(1, 10) // Frame constant buffer
        .AddSamplerDescriptorTableParameter(0, 10, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize) // Sampler[]
        .AddSamplerDescriptorTableParameter(0, 11, D3D12_SHADER_VISIBILITY_ALL, RootSignatureBuilder::UnboundedRangeSize); // SamplerComparisonState[]
    }
} // namespace Engine::Render