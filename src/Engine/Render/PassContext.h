#pragma once

#include <Types.h>
#include <Render/ResourceStateTracker.h>
#include <Render/RenderContext.h>
#include <Render/FrameResourceProvider.h>
#include <Render/FrameTransientContext.h>

#include <Scene/SceneObject.h>
#include <Timer.h>

namespace Engine::Render
{
    class PassContext
    {
    public:
        SharedPtr<ResourceStateTracker> resourceStateTracker;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        SharedPtr<RenderContext> renderContext;

        const FrameResourceProvider* frameResourceProvider;

        FrameTransientContext  * frameContext;

        Scene::SceneObject * scene;

        const Timer * timer;
    };
} // namespace Engine::Render
