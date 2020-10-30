#pragma once

#include <entt/entt.hpp>

namespace Engine::UI::ComponentRenderers
{
    class ComponentRendererBase
    {
        public:
            ComponentRendererBase() {}
            virtual ~ComponentRendererBase(){}

            virtual void RenderComponent(entt::registry& registry, entt::entity entity) = 0;
    };

    template <typename TComponent>
    class ComponentRenderer : public ComponentRendererBase
    {
        public:

            void RenderComponent(entt::registry& registry, entt::entity entity) override
            {
                auto name = typeid(TComponent).name();
                if (auto* componentPtr = registry.try_get<TComponent>(entity))
                {
                    RenderComponent(registry, entity, *componentPtr);
                }
            }
        protected:
            virtual void RenderComponent(entt::registry& registry, entt::entity entity, TComponent& component) = 0;
    };
}