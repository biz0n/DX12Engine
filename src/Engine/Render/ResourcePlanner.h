#pragma once

#include <Types.h>

#include <Name.h>
#include <Render/RenderForwards.h>
#include <Render/TextureCreationInfo.h>

#include <d3d12.h>
#include <vector>

namespace Engine::Render
{
    class ResourcePlanner
    {   
        public:
            struct ResourceCreationInfo
            {
                Name name;
                TextureCreationInfo creationInfo;
            };
            
        public:
            ResourcePlanner();
            ~ResourcePlanner();
        
        public:

            void NewRenderTarget(const Name& name, const TextureCreationInfo& textureInfo);
            void NewDepthStencil(const Name& name, const TextureCreationInfo& textureInfo);

            void ReadRenderTarget(const Name& name);
            void ReadDeptStencil(const Name& name);

            const std::vector<ResourceCreationInfo> GetPlannedResources() const { return mResourcesForCreate; }
        
        private:
            std::vector<ResourceCreationInfo> mResourcesForCreate;
    };
} // namespace Engine::Render
