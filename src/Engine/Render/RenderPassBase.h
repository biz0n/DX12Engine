#pragma once

#include <Types.h>

#include <Render/ResourcePlanner.h>
#include <Render/RootSignatureProvider.h>
#include <Render/PipelineStateProvider.h>
#include <Render/PassContext.h>

namespace Engine::Render
{
    class RenderPassBase
    {
    public:
        RenderPassBase(const std::string& passName) : mPassName {passName} {}
        virtual ~RenderPassBase() {}

        virtual void PrepareResources(Render::ResourcePlanner* planner) {}

        virtual void CreateRootSignatures(Render::RootSignatureProvider* rootSignatureProvider) {}

        virtual void CreatePipelineStates(Render::PipelineStateProvider* pipelineStateProvider) {}

        virtual void Render(Render::PassContext& passContext) {}

        const std::string& GetName() const { return mPassName; }
    private:
        std::string mPassName;
    };
} // namespace Engine::Render
