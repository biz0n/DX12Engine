#pragma once

#include <Types.h>
#include <Name.h>
#include <Render/RootSignature.h>
#include <Render/RootSignatureBuilder.h>

#include <d3d12.h>
#include <unordered_map>

namespace Engine::Render
{
    class RootSignatureProvider
    {
    private:
        std::unordered_map<Name, UniquePtr<RootSignature>> mRootSignatureMap;
        ComPtr<ID3D12Device2> mDevice;
    public:
        RootSignatureProvider(ComPtr<ID3D12Device2> device);
        ~RootSignatureProvider();

        void BuildRootSignature(const Name& name, RootSignatureBuilder& builder);

        RootSignature* GetRootSignature(const Name& name);
    };
    
    
} // namespace Engine::Render
