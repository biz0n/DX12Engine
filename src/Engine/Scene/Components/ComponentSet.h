#pragma once

#include <Scene/Components/AABBComponent.h>
#include <Scene/Components/CameraComponent.h>
#include <Scene/Components/IsDisabledComponent.h>
#include <Scene/Components/LightComponent.h>
#include <Scene/Components/LocalTransformComponent.h>
#include <Scene/Components/MeshComponent.h>
#include <Scene/Components/MovingComponent.h>
#include <Scene/Components/NameComponent.h>
#include <Scene/Components/RelationshipComponent.h>
#include <Scene/Components/WorldTransformComponent.h>

#include <string_view>

namespace Engine::Scene::Components
{
    template <class...TComponents> struct ComponentSet {};
    using AllComponents = ComponentSet<
        AABBComponent, CameraComponent, IsDisabledComponent, LightComponent, 
        MeshComponent, MovingComponent, NameComponent, RelationshipComponent, 
        LocalTransformComponent, WorldTransformComponent>;

    template<class T>
    constexpr static std::string_view GetComponentName()
    {
        std::string_view fullName = typeid(T).name();
        size_t index = fullName.find_last_of(':');
        return fullName.substr(index + 1);
    }
}