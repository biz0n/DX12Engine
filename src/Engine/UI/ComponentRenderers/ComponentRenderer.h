#pragma once

#include <entt/entt.hpp>

namespace Engine::UI::ComponentRenderers
{
    class ComponentRendererBase
    {
        public:
            ComponentRendererBase(const std::string& name) : mName(name) {}
            virtual ~ComponentRendererBase(){}

            virtual bool HasComponent(entt::registry& registry, entt::entity entity) const = 0;
            virtual void RenderComponent(entt::registry& registry, entt::entity entity) = 0;
            const std::string& Name() const { return mName; }
        private:
            std::string mName;
    };

    template <typename TComponent>
    class ComponentRenderer : public ComponentRendererBase
    {
        public:
            ComponentRenderer(const std::string& name) : ComponentRendererBase(name) {}

            bool HasComponent(entt::registry& registry, entt::entity entity) const override
            {
                return registry.has<TComponent>(entity);
            }

            void RenderComponent(entt::registry& registry, entt::entity entity) override
            {
                auto& component = registry.get<TComponent>(entity);
                
                RenderComponent(registry, entity, component);
            }
        protected:
            virtual void RenderComponent(entt::registry& registry, entt::entity entity, TComponent& component) = 0;
    };
}