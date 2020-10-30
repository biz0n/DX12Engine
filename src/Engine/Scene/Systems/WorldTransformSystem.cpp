#include "WorldTransformSystem.h"

#include <entt/entt.hpp>

#include <DirectXMath.h>
#include <queue>

namespace Engine::Scene::Systems
{
    WorldTransformSystem::WorldTransformSystem() : System()
    {
    }

    WorldTransformSystem::~WorldTransformSystem() = default;

    void WorldTransformSystem::Init(entt::registry *registry)
    {
        registry->on_construct<Components::LocalTransformComponent>().connect<&WorldTransformSystem::InitWithDirty>(this);
        registry->on_construct<Components::RelationshipComponent>().connect<&WorldTransformSystem::InitWithDirty>(this);

        registry->on_update<Components::LocalTransformComponent>().connect<&WorldTransformSystem::MarkAsDirty>(this);

        auto& view = registry->view<Components::RelationshipComponent, Components::LocalTransformComponent>();
        for (auto e : view)
        {
            registry->emplace_or_replace<Components::Dirty>(e);
        }
    }

    void WorldTransformSystem::Process(entt::registry *registry, const Timer &timer)
    {
        auto group = registry->group<Scene::Components::Dirty, Scene::Components::LocalTransformComponent, Scene::Components::RelationshipComponent>();
        group.sort([this, &registry](const entt::entity left, const entt::entity right) 
        {
            const auto &leftRshp = registry->get<Scene::Components::RelationshipComponent>(left);
            const auto &rightRshp = registry->get<Scene::Components::RelationshipComponent>(right);
            
            return left == rightRshp.Parent 
                   || 
                   leftRshp.Next == right 
                   || 
                   (
                       !(leftRshp.Parent == right || rightRshp.Next == left) 
                       && 
                       (
                           (leftRshp.Parent == rightRshp.Parent && left < right)
                           ||
                           (leftRshp.Parent == entt::null && rightRshp.Parent != entt::null)
                           ||
                           ((leftRshp.Parent < rightRshp.Parent && rightRshp.Parent != entt::null))
                       )
                   );
        });

        for (auto e : group)
        {
            const auto& local = registry->get<Scene::Components::LocalTransformComponent>(e);
            const auto& relationship = registry->get<Scene::Components::RelationshipComponent>(e);

            DirectX::XMMATRIX worldTransform;
            if (relationship.Parent == entt::null)
            {
                worldTransform = local.transform;                
            }
            else
            {
                const auto& parentWorld = registry->get<Scene::Components::WorldTransformComponent>(relationship.Parent);

                worldTransform = DirectX::XMMatrixMultiply(parentWorld.transform, local.transform);
            }

            registry->emplace_or_replace<Scene::Components::WorldTransformComponent>(e, worldTransform);
            registry->remove<Scene::Components::Dirty>(e);
        }

        registry->remove<Scene::Components::Dirty>(group.begin(), group.end());
    }

    void WorldTransformSystem::InitWithDirty(entt::registry& r, entt::entity entity)
    {
        if (r.has<Components::RelationshipComponent, Components::LocalTransformComponent>(entity))
        {
            r.emplace_or_replace<Components::Dirty>(entity);
        }
    }

    void WorldTransformSystem::MarkAsDirty(entt::registry& r, entt::entity entity)
    {
        if (auto* relationshipPtr = r.try_get<Components::RelationshipComponent>(entity); relationshipPtr)
        {
            std::queue<std::tuple<entt::entity, Components::RelationshipComponent>> entities;
            auto relationship = *relationshipPtr;

            entities.push(std::make_tuple(entity, relationship));

            while(!entities.empty())
            {
                auto e = entities.front();
                entities.pop();

                r.emplace_or_replace<Components::Dirty>(std::get<0>(e));
                relationship = std::get<1>(e);

                auto child = relationship.First;
                while(child != entt::null)
                {
                    relationship = r.get<Components::RelationshipComponent>(child);
                    entities.push(std::make_tuple(child, relationship));

                    child = relationship.Next;
                }
            }
        }
    }

} // namespace Engine::Scene::Systems