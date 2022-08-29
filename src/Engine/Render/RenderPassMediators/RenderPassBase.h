#pragma once

#include <Types.h>
#include <Timer.h>

#include <Render/RenderForwards.h>

#include <utility>

namespace Engine::Render
{
    enum class CommandQueueType
    {
        Graphics = 0,
        Compute = 1
    };

    class RenderPassBase
    {
    public:
        RenderPassBase(std::string passName, CommandQueueType queueType)
        : mPassName{std::move(passName)}, mCommandQueueType{queueType}
        {

        }
        virtual ~RenderPassBase() = default;

        virtual void PrepareResources(Render::ResourcePlanner *planner) {}

        virtual void CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider) {}

        virtual void CreatePipelineStates(Render::PipelineStateProvider *pipelineStateProvider) {}

        virtual void Render(const RenderRequest& renderRequest, Render::PassRenderContext &passContext, const Timer &timer) {}

        const std::string &GetName() const { return mPassName; }
        CommandQueueType GetQueueType() const { return mCommandQueueType; }

    private:
        std::string mPassName;
        CommandQueueType mCommandQueueType;
    };
} // namespace Engine::Render
