#pragma once

#include <Types.h>

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

        virtual void Render(Render::PassRenderContext &passContext) {}

        const std::string &GetName() const { return mPassName; }
        CommandQueueType GetQueueType() const { return mCommandQueueType; }

    private:
        std::string mPassName;
        CommandQueueType mCommandQueueType;
    };

    template <class TPassData>
    class RenderPassBaseWithData : public RenderPassBase
    {
    public:
        RenderPassBaseWithData(const std::string &passName, CommandQueueType queueType) : RenderPassBase(passName, queueType), mPassData{} {}
        ~RenderPassBaseWithData() override = default;

        const TPassData &PassData() const { return mPassData; }

        void SetPassData(const TPassData &passData) { mPassData = passData; }

    private:
        TPassData mPassData;
    };
} // namespace Engine::Render
