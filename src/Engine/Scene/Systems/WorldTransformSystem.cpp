#include "WorldTransformSystem.h"

#include <entt/entt.hpp>

#include <DirectXMath.h>

namespace Engine::Scene::Systems
{
    WorldTransformSystem::WorldTransformSystem() : System()
    {
    }

    WorldTransformSystem::~WorldTransformSystem() = default;

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

            if (!registry->has<Scene::Components::Dirty>(e))
            {
                continue;
            }

            DirectX::XMMATRIX worldTransform;
            if (relationship.Parent == entt::null)
            {
                worldTransform = local.Transform;                
            }
            else
            {
                const auto& parentWorld = registry->get<Scene::Components::WorldTransformComponent>(relationship.Parent);

                worldTransform = DirectX::XMMatrixMultiply(parentWorld.Transform, local.Transform);
            }

            registry->emplace_or_replace<Scene::Components::WorldTransformComponent>(e, worldTransform);
            registry->remove<Scene::Components::Dirty>(e);
        }

        registry->remove<Scene::Components::Dirty>(group.begin(), group.end());
    }

} // namespace Engine::Scene::Systems