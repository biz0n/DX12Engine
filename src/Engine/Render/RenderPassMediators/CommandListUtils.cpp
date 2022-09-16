#include "CommandListUtils.h"

#include <Exceptions.h>
#include <d3dx12.h>

#include <Memory/ResourceStateTracker.h>
#include <Memory/IndexBuffer.h>
#include <Memory/VertexBuffer.h>
#include <Memory/UploadBuffer.h>
#include <Memory/Texture.h>

#include <Render/RenderContext.h>
#include <Render/RenderPassMediators/PassCommandRecorder.h>

#include <Scene/Image.h>
#include <Scene/Material.h>

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