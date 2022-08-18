#pragma once

#include <Types.h>

#include <Name.h>
#include <Render/RenderForwards.h>
#include <Memory/TextureCreationInfo.h>
#include <Graph/GraphForwards.h>

#include <d3d12.h>
#include <vector>
#include <unordered_map>

namespace Engine::Render
{
    struct NewTextureParameters
    {
        D3D12_RESOURCE_DIMENSION Dimension;
        UINT64 Width;
        UINT Height;
        UINT16 DepthOrArraySize;
        UINT16 MipLevels;
        DXGI_FORMAT Format;
    };

    class ResourcePlanner
    {   
        public:
            struct ResourceCreationInfo
            {
                Name name;
                Memory::TextureCreationInfo creationInfo;
                D3D12_RESOURCE_STATES state;
            };
        public:
            ResourcePlanner(Engine::Graph::Node* renderNode, SharedPtr<FrameResourceProvider> frameResourceProvider);
            ~ResourcePlanner();
        
        public:
            void NewRenderTarget(const Name& name, const Memory::TextureCreationInfo& textureInfo);
            void NewDepthStencil(const Name& name, const Memory::TextureCreationInfo& textureInfo);
            void NewTexture(const Name& name, const Memory::TextureCreationInfo& textureInfo);
            //void NewBuffer(const Name& name);

            void WriteRenderTarget(const Name& name, const Name& originalName);
            void WriteDepthStencil(const Name& name, const Name& originalName);
            void WriteTexture(const Name& name, const Name& originalName);
            //void WriteBuffer(const Name& name, const Name& originalName);

            void ReadRenderTarget(const Name& name);
            void ReadDeptStencil(const Name& name);
            void ReadTexture(const Name& name);
            //void ReadBuffer(const Name& name);

        
        private:
            SharedPtr<FrameResourceProvider> mFrameResourcesProvider;
            Engine::Graph::Node* mRenderNode;
    };
} // namespace Engine::Render
