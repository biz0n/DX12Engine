#pragma once

#include <Types.h>

#include <Render/RenderForwards.h>
#include <Render/RenderPassMediators/ResourcePlanner.h>
#include <Render/RenderPassMediators/RenderPassBase.h>
#include <Graph/GraphForwards.h>

namespace Engine::Render
{
    class PassContext
    {
        public:
            PassContext(RenderPassBase* renderPass, Graph::Node* renderNode, SharedPtr<FrameResourceProvider> frameResourceProvider)
                : mRenderPass{renderPass}, mResourcePlanner{renderNode, frameResourceProvider }
            {
                mCommandQueueType = renderPass->GetQueueType();
            }

            CommandQueueType GetQueueType() const { return mCommandQueueType; }

            RenderPassBase* GetRenderPass() const { return mRenderPass; }

            ResourcePlanner* GetResourcePlanner() { return &mResourcePlanner; }

        private:
            ResourcePlanner mResourcePlanner;

            RenderPassBase* mRenderPass;

            CommandQueueType mCommandQueueType;
    };
}