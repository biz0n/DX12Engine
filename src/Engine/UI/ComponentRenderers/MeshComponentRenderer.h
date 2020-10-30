#pragma once

#include <Types.h>

#include <RenderContext.h>
#include <UIRenderContext.h>

#include <UI/ComponentRenderers/ComponentRenderer.h>

#include <Scene/Components/MeshComponent.h>
#include <Scene/Mesh.h>
#include <Scene/Material.h>

#include <entt/entt.hpp>
#include <imgui/imgui.h>


namespace Engine::UI::ComponentRenderers
{
    
    class MeshComponentRenderer : public ComponentRenderer<Engine::Scene::Components::MeshComponent>
    {
        private:
            SharedPtr<RenderContext> mRenderContext;
            SharedPtr<UIRenderContext> mUIRenderContext;
        public:
            MeshComponentRenderer(SharedPtr<RenderContext> renderContext, SharedPtr<UIRenderContext> uiRenderContext)
                : mRenderContext(renderContext), mUIRenderContext(uiRenderContext)
             {}
            ~MeshComponentRenderer() {}

        protected:
            void RenderComponent(entt::registry& registry, entt::entity entity, Engine::Scene::Components::MeshComponent& meshComponent) override
            {

                if (meshComponent.mesh.material->HasBaseColorTexture())
                {
                    auto texture = meshComponent.mesh.material->GetBaseColorTexture();

                    auto srv = texture->GetShaderResourceView(
                        mRenderContext->Device(),
                        mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    auto textureId = mUIRenderContext->GetTextureId(srv);

                    ImGui::Image(textureId, {256, 256});
                }

                if (meshComponent.mesh.material->HasNormalTexture())
                {
                    auto texture = meshComponent.mesh.material->GetNormalTexture();

                    auto srv = texture->GetShaderResourceView(
                        mRenderContext->Device(),
                        mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    auto textureId = mUIRenderContext->GetTextureId(srv);

                    ImGui::Image(textureId, {256, 256});
                }

                if (meshComponent.mesh.material->HasMetallicRoughnessTexture())
                {
                    auto texture = meshComponent.mesh.material->GetMetallicRoughnessTexture();

                    auto srv = texture->GetShaderResourceView(
                        mRenderContext->Device(),
                        mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    auto textureId = mUIRenderContext->GetTextureId(srv);

                    ImGui::Image(textureId, {256, 256});
                }

                if (meshComponent.mesh.material->HasEmissiveTexture())
                {
                    auto texture = meshComponent.mesh.material->GetEmissiveTexture();

                    auto srv = texture->GetShaderResourceView(
                        mRenderContext->Device(),
                        mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    auto textureId = mUIRenderContext->GetTextureId(srv);

                    ImGui::Image(textureId, {256, 256});
                }

                if (meshComponent.mesh.material->HasAmbientOcclusionTexture())
                {
                    auto texture = meshComponent.mesh.material->GetAmbientOcclusionTexture();

                    auto srv = texture->GetShaderResourceView(
                        mRenderContext->Device(),
                        mRenderContext->GetDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
                    auto textureId = mUIRenderContext->GetTextureId(srv);

                    ImGui::Image(textureId, {256, 256});
                }
            }
    };
}