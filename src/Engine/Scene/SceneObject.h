#pragma once

#include <Types.h>
#include <Timer.h>
#include <Scene/SceneForwards.h>

#include <Scene/Systems/System.h>
#include <vector>
#include <queue>
#include <tuple>
#include <entt/fwd.hpp>

#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/LocalTransformComponent.h>

namespace Engine::Scene
{
	class SceneObject
	{
	public:
		SceneObject()
		{
			registry = MakeUnique<entt::registry>();

			registry->on_construct<Components::LocalTransformComponent>().connect<&entt::registry::emplace_or_replace<Components::Dirty>>();

			registry->on_update<Components::LocalTransformComponent>().connect<&SceneObject::MarkAsDirty>(this);
		}

	public:
		std::vector<SharedPtr<MeshNode>> nodes;

		std::vector<SharedPtr<LightNode>> lights;

		std::vector<SharedPtr<CameraNode>> cameras;

		UniquePtr<entt::registry> registry;

	public:
		void AddSystem(UniquePtr<Systems::System> system)
		{
			mSystems.push_back(std::move(system));
		}

		void Process(const Timer& timer)
		{
			for (int i = 0; i < mSystems.size(); ++i)
			{
				mSystems[i]->Process(registry.get(), timer);
			}
		}
	private:
		void MarkAsDirty(entt::registry& r, entt::entity entity)
		{
			std::queue<std::tuple<entt::entity, Components::RelationshipComponent>> entities;
			auto relationship = r.get<Components::RelationshipComponent>(entity);

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

	private:
		std::vector<UniquePtr<Systems::System>> mSystems;
	};
}