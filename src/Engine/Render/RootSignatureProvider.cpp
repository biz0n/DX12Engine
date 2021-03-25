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

        auto rs = builder.Build(mDevice);
        rs->GetD3D12RootSignature()->SetName(StringToWString(name.string()).c_str());
        mRootSignatureMap.emplace(name, std::move(rs));
    }

    RootSignature *RootSignatureProvider::GetRootSignature(const Name &name)
    {
        return mRootSignatureMap[name].get();
    }

} // namespace Engine::Render