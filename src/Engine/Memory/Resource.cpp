#include "Resource.h"
#include <HAL/DirectXExtensions.h>

namespace Engine::Memory
{
    Resource::Resource() : mResource(nullptr)
    {
    }

    Resource::~Resource()
    {
    }


    void Resource::SetName(const std::string &name)
    {
        if (mResource)
        {
            Engine::HAL::SetResourceName(mResource, name);
        }
        mResourceName = name;
    }
} // namespace Engine::Memory