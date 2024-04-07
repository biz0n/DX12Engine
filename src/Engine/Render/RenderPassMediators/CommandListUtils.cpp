#include "CommandListUtils.h"

#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/ResourceStateTracker.h>

#include <Render/RenderContext.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>

#include <DirectXTex.h>
#include <DirectXMath.h>

#include <filesystem>
#include <map>

namespace Engine::Render::CommandListUtils
{
    Shader::FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount)
    {
        Shader::FrameUniform cb = {};
        DirectX::XMStoreFloat4x4(&cb.ViewProj, viewProj);

        dx::XMStoreFloat3(&cb.EyePos, eyePos);

        cb.LightsCount = lightsCount;

        using namespace DirectX;
        DirectX::XMVECTOR planes[6] =
        {
            DirectX::XMPlaneNormalize(viewProj.r[3] + viewProj.r[0]), // Left
            DirectX::XMPlaneNormalize(viewProj.r[3] - viewProj.r[0]), // Right
            DirectX::XMPlaneNormalize(viewProj.r[3] + viewProj.r[1]), // Bottom
            DirectX::XMPlaneNormalize(viewProj.r[3] - viewProj.r[1]), // Top
            DirectX::XMPlaneNormalize(viewProj.r[2]),           // Near
            DirectX::XMPlaneNormalize(viewProj.r[3] - viewProj.r[2]), // Far
        };

        for (uint32_t i = 0; i < _countof(planes); ++i)
        {
            DirectX::XMStoreFloat4(&cb.Planes[i], planes[i]);
        }

        return cb;
    }

    void TransitionBarrier(Memory::ResourceStateTracker* stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            resource,
            D3D12_RESOURCE_STATE_COMMON,
            targetState);
        stateTracker->ResourceBarrier(barrier);
    }
} // namespace Engine::CommandListUtils