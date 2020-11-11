#include "RootSignatureProvider.h"

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
        
        mRootSignatureMap.emplace(name, std::move(builder.Build(mDevice)));
    }

    RootSignature *RootSignatureProvider::GetRootSignature(const Name &name)
    {
        return mRootSignatureMap[name].get();
    }

} // namespace Engine::Render