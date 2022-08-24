#pragma once

#include <Types.h>
#include <Render/ShaderTypes.h>

#include <Scene/SceneForwards.h>
#include <Render/RenderForwards.h>
#include <Memory/MemoryForwards.h>

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace Engine::Render::CommandListUtils
{
    Shader::FrameUniform GetFrameUniform(const DirectX::XMMATRIX& viewProj, const DirectX::XMVECTOR& eyePos, uint32 lightsCount);

    void TransitionBarrier(Memory::ResourceStateTracker* stateTracker, ID3D12Resource* resource, D3D12_RESOURCE_STATES targetState);
} // namespace Engine::RenderUtils