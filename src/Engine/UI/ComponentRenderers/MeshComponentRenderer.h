#pragma once

#include <Types.h>

#include <Render/RenderContext.h>
#include <Render/UIRenderContext.h>

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
            SharedPtr<Render::RenderContext> mRenderContext;
            SharedPtr<Render::UIRenderContext> mUIRenderContext;
        public:
            MeshComponentRenderer(SharedPtr<Render::RenderContext> renderContext, SharedPtr<Render::UIRenderContext> uiRenderContext)
                : ComponentRenderer("Mesh Component"), mRenderContext(renderContext), mUIRenderContext(uiRenderContext)
             {}
            ~MeshComponentRenderer() override = default;

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