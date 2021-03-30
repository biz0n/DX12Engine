#pragma once

#include <Types.h>

#include <Render/RenderForwards.h>
#include <Render/ResourcePlanner.h>

namespace Engine::Render
{
    enum class CommandQueueType
    {
        Graphics = 0,
        Compute = 1
    };

    class PassContext
    {
        public:
            PassContext(RenderPassBase* renderPass) : mRenderPass{renderPass}
            {

            }

        private:
            ResourcePlanner mResourcePlanner;

            RenderPassBase* mRenderPass;
    };
}