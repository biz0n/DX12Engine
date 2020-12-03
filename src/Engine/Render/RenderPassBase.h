#pragma once

#include <Types.h>

#include <Render/RenderForwards.h>

namespace Engine::Render
{
    class RenderPassBase
    {
    public:
        RenderPassBase(const std::string &passName) : mPassName{passName} {}
        virtual ~RenderPassBase() {}

        virtual void PrepareResources(Render::ResourcePlanner *planner) {}

        virtual void CreateRootSignatures(Render::RootSignatureProvider *rootSignatureProvider) {}

        virtual void CreatePipelineStates(Render::PipelineStateProvider *pipelineStateProvider) {}

        virtual void Render(Render::PassContext &passContext) {}

        const std::string &GetName() const { return mPassName; }

    private:
        std::string mPassName;
    };

    template <class TPassData>
    class RenderPassBaseWithData : public RenderPassBase
    {
    public:
        RenderPassBaseWithData(const std::string &passName) : RenderPassBase(passName), mPassData{} {}
        virtual ~RenderPassBaseWithData() {}

        const TPassData &PassData() const { return mPassData; }

        void SetPassData(const TPassData &passData) { mPassData = passData; }

    private:
        TPassData mPassData;
    };
} // namespace Engine::Render
