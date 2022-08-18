#pragma once

#include <Types.h>

#include <Render/RenderForwards.h>
#include <HAL/RootSignature.h>

#include <vector>
#include <tuple>
#include <optional>

#include <d3d12.h>
#include <d3dx12.h>

namespace Engine::Render
{
    class RootSignatureBuilder
    {
    public:
        inline const static uint32 UnboundedRangeSize = std::numeric_limits<uint32>::max();
    public:
        RootSignatureBuilder();
        ~RootSignatureBuilder();

        template <typename TConstantsType>
        RootSignatureBuilder& AddConstantsParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
        {
            CD3DX12_ROOT_PARAMETER1 parameter;
            auto num32BitValues = static_cast<uint32>(sizeof(TConstantsType) / 4);
            parameter.InitAsConstants(num32BitValues, registerIndex, registerSpace, visibility);

            mParameters.push_back({ parameter, std::nullopt });

            return *this;
        }

        RootSignatureBuilder& AddCBVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        RootSignatureBuilder& AddSRVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        RootSignatureBuilder& AddUAVParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
        RootSignatureBuilder& AddSRVDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, uint32 numDescriptors = 1);
        RootSignatureBuilder& AddUAVDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, uint32 numDescriptors = 1);
        RootSignatureBuilder& AddSamplerDescriptorTableParameter(uint32 registerIndex, uint32 registerSpace, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL, uint32 numDescriptors = 1);

        UniquePtr<HAL::RootSignature> Build(ComPtr<ID3D12Device2> device);
    private:
        std::vector<std::tuple<CD3DX12_ROOT_PARAMETER1, std::optional<CD3DX12_DESCRIPTOR_RANGE1>>> mParameters;
    };
}