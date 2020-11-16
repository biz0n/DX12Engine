#include "SceneObject.h"

namespace Engine::Scene
{
    SceneObject::SceneObject() = default;

    SceneObject::~SceneObject() = default;

    entt::registry& SceneObject::GetRegistry() 
    { 
        return registry; 
    }

    void SceneObject::AddSystem(UniquePtr<Systems::System> system)
    {
        system->Init(this);
        mSystems.push_back(std::move(system));
    }

    void SceneObject::Process(const Timer &timer)
    {
        for (int i = 0; i < mSystems.size(); ++i)
        {
            mSystems[i]->Process(this, timer);
        }
    }
} // namespace Engine::Scene