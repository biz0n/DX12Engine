#pragma once

#include <Types.h>
#include <Name.h>
#include <Render/RenderForwards.h>
#include <HAL/HALForwards.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class RootSignatureProvider
    {
    private:
        std::unordered_map<Name, UniquePtr<HAL::RootSignature>> mRootSignatureMap;
        ComPtr<ID3D12Device2> mDevice;
    public:
        RootSignatureProvider(ComPtr<ID3D12Device2> device);
        ~RootSignatureProvider();

        void BuildRootSignature(const Name& name, RootSignatureBuilder& builder);

        HAL::RootSignature* GetRootSignature(const Name& name);
    private:
        void AddDefaultRegisters(RootSignatureBuilder& builder);
    };
    
    
} // namespace Engine::Render
